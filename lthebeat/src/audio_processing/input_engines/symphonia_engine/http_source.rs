use crate::audio_processing::input_engines::symphonia_engine::ExternalMetadata;
use isahc::config::{Configurable, RedirectPolicy};
use isahc::http::Uri;
use isahc::{Request, RequestExt};
use log::warn;
use rb::{RB, RbConsumer, RbInspector, RbProducer, SpscRb};
use regex::Regex;
use smol::io::AsyncReadExt;
use std::future::pending;
use std::io::{Read, Seek, SeekFrom};
use std::mem;
use std::str::FromStr;
use std::sync::{Arc, LazyLock, RwLock};
use symphonia::core::io::MediaSource;
use symphonia::core::meta::{
    MetadataBuilder, MetadataLog, MetadataRevision, StandardTagKey, Tag, Value,
};
use tracing::{debug, error, info};
use url::Url;

const HTTP_SOURCE_BUFFER_SIZE: usize = 1048576 * 32;

struct HttpSourcePendingMetadataRevision {
    bytes_remaining: usize,
    metadata_revision: MetadataRevision,
}

pub struct HttpSource {
    url: Url,
    rb: Arc<SpscRb<u8>>,
    content_length: Option<u64>,
    metadata_log: Arc<RwLock<MetadataLog>>,
    pending_metadata_revisions: Arc<RwLock<Vec<HttpSourcePendingMetadataRevision>>>,
}

pub static ICECAST_EXTRACTION_REGEX: LazyLock<Regex> =
    LazyLock::new(|| Regex::new(r"(?<key>.+?)='(?<value>.+?)';").unwrap());

impl HttpSource {
    pub fn new(url: Url) -> HttpSource {
        let mut metadata_log: MetadataLog = Default::default();
        metadata_log.push(Default::default());
        let mut source = HttpSource {
            url,
            rb: Arc::new(SpscRb::new(HTTP_SOURCE_BUFFER_SIZE)),
            content_length: None,
            metadata_log: Arc::new(RwLock::new(metadata_log)),
            pending_metadata_revisions: Arc::new(RwLock::new(Vec::new())),
        };
        source.start_stream();
        source
    }

    pub fn start_stream(&mut self) {
        let url = self.url.clone();
        let rb = self.rb.clone();
        let producer = self.rb.producer();
        let pending_metadata_revisions = self.pending_metadata_revisions.clone();

        smol::spawn(async move {
            // TODO: HEAD request to get stream data
            // Pull out Content-Length and allocate a temp file that large
            // (if not present or > 100 MB, just stream the data)
            let response = Request::get(Uri::from_str(url.clone().as_str()).unwrap())
                .redirect_policy(RedirectPolicy::Follow)
                .header("Icy-Metadata", "1")
                .body(())
                .unwrap()
                .send_async()
                .await;

            let Ok(response) = response else {
                return;
            };

            let icecast_metadata_frequency = response
                .headers()
                .get("icy-metaint")
                .and_then(|v| v.to_str().ok())
                .and_then(|v| usize::from_str(v).ok());
            let mut bytes_until_icecast_meta_packet = icecast_metadata_frequency.unwrap_or(0);

            let mut response = response.into_body();

            let process_icecast_metadata = |metadata: &[u8]| {
                if !metadata.is_empty() {
                    debug!("Icecast metadata: {}", String::from_utf8_lossy(metadata));

                    let metadata_update = String::from_utf8_lossy(metadata);
                    let captures = ICECAST_EXTRACTION_REGEX.captures_iter(&metadata_update);

                    for capture in captures {
                        let key = &capture["key"];
                        let value = &capture["value"];

                        let mut metadata_builder = MetadataBuilder::new();
                        metadata_builder.add_tag(Tag::new(
                            match key {
                                "StreamTitle" => Some(StandardTagKey::TrackTitle),
                                _ => None,
                            },
                            key,
                            Value::String(value.to_string()),
                        ));

                        pending_metadata_revisions.write().unwrap().push(
                            HttpSourcePendingMetadataRevision {
                                bytes_remaining: rb.count(),
                                metadata_revision: metadata_builder.metadata(),
                            },
                        )
                    }
                }
            };

            let process_stream_data = |stream_data: &[u8]| {
                if rb.slots_free() >= stream_data.len() {
                    producer.write(stream_data).unwrap();
                    true
                } else {
                    false
                }
            };

            loop {
                let mut buf = vec![0; 2048];
                let Ok(result) = response.read(&mut buf).await else {
                    break;
                };

                buf.truncate(result);

                // Process icecast metadata
                if let Some(icecast_metadata_frequency) = icecast_metadata_frequency {
                    if bytes_until_icecast_meta_packet < buf.len() {
                        let stream_data = &buf[..bytes_until_icecast_meta_packet];

                        if !process_stream_data(stream_data) {
                            // TODO: Restart the stream when the buffer is empty
                            warn!("HttpSource: Buffer is full, stopping download");
                            drop(response);
                            break;
                        }

                        // Start processing icecast metadata
                        let icecast_data_packet_length =
                            buf[bytes_until_icecast_meta_packet] as usize * 16;
                        if bytes_until_icecast_meta_packet + icecast_data_packet_length > buf.len()
                        {
                            let remaining_icecast_data_bytes = buf.len()
                                - bytes_until_icecast_meta_packet
                                + icecast_data_packet_length;
                            let old_len = buf.len();

                            // Read in the remaining bytes of the metadata packet
                            buf.resize(old_len + remaining_icecast_data_bytes, 0);
                            let Ok(result) = response.read(&mut buf[old_len..]).await else {
                                error!("HttpSource: Error reading icecast metadata");
                                break;
                            };
                        }

                        let icecast_meta_packet_end =
                            bytes_until_icecast_meta_packet + icecast_data_packet_length;

                        if icecast_data_packet_length > 0 {
                            let icecast_metadata_buf = &buf
                                [(bytes_until_icecast_meta_packet + 1)..icecast_meta_packet_end];
                            process_icecast_metadata(icecast_metadata_buf);
                        }

                        bytes_until_icecast_meta_packet = icecast_metadata_frequency;

                        let remaining_stream = &buf[(icecast_meta_packet_end + 1)..];
                        if !remaining_stream.is_empty() {
                            if !process_stream_data(remaining_stream) {
                                // TODO: Restart the stream when the buffer is empty
                                warn!("HttpSource: Buffer is full, stopping download");
                                drop(response);
                                break;
                            }
                            bytes_until_icecast_meta_packet -= remaining_stream.len();
                        }
                    } else {
                        if !process_stream_data(&buf) {
                            // TODO: Restart the stream when the buffer is empty
                            warn!("HttpSource: Buffer is full, stopping download");
                            drop(response);
                            break;
                        }
                        bytes_until_icecast_meta_packet -= buf.len();
                    }
                } else if !process_stream_data(&buf) {
                    // TODO: Restart the stream when the buffer is empty
                    warn!("HttpSource: Buffer is full, stopping download");
                    drop(response);
                    break;
                }
            }
        })
            .detach();
    }
}

impl Read for HttpSource {
    fn read(&mut self, buf: &mut [u8]) -> std::io::Result<usize> {
        match self.rb.consumer().read_blocking(buf) {
            Some(bytes_read) => {
                let mut pending_metadata_revisions =
                    self.pending_metadata_revisions.write().unwrap();

                let mut pending_metadata_log_entries = Vec::new();
                pending_metadata_revisions.retain_mut(|revision| {
                    if revision.bytes_remaining > bytes_read {
                        revision.bytes_remaining -= bytes_read;
                        true
                    } else {
                        pending_metadata_log_entries
                            .push(mem::take(&mut revision.metadata_revision));
                        false
                    }
                });

                let mut metadata_log = self.metadata_log.write().unwrap();
                for entry in pending_metadata_log_entries {
                    metadata_log.push(entry);
                }

                Ok(bytes_read)
            }
            _ => Ok(0),
        }
    }
}

impl Seek for HttpSource {
    fn seek(&mut self, _: SeekFrom) -> std::io::Result<u64> {
        panic!("Unable to seek HTTP source")
    }
}

impl MediaSource for HttpSource {
    fn is_seekable(&self) -> bool {
        false
    }

    fn byte_len(&self) -> Option<u64> {
        self.content_length
    }
}

impl ExternalMetadata for HttpSource {
    fn metadata_log(&self) -> Arc<RwLock<MetadataLog>> {
        self.metadata_log.clone()
    }
}
