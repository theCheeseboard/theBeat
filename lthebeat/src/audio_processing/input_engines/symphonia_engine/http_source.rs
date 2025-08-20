use isahc::config::{Configurable, RedirectPolicy};
use isahc::http::Uri;
use isahc::{Request, RequestExt};
use log::warn;
use rb::{RB, RbConsumer, RbInspector, RbProducer, SpscRb};
use smol::io::AsyncReadExt;
use std::io::{Read, Seek, SeekFrom};
use std::str::FromStr;
use std::sync::Arc;
use symphonia::core::io::MediaSource;
use tracing::debug;
use url::Url;

const HTTP_SOURCE_BUFFER_SIZE: usize = 1048576 * 32;

pub struct HttpSource {
    url: Url,
    rb: Arc<SpscRb<u8>>,
    content_length: Option<u64>,
}

impl HttpSource {
    pub fn new(url: Url) -> HttpSource {
        let mut source = HttpSource {
            url,
            rb: Arc::new(SpscRb::new(HTTP_SOURCE_BUFFER_SIZE)),
            content_length: None,
        };
        source.start_stream();
        source
    }

    pub fn start_stream(&mut self) {
        let url = self.url.clone();
        let rb = self.rb.clone();
        let producer = self.rb.producer();
        smol::spawn(async move {
            // TODO: HEAD request to get stream data
            // Pull out Content-Length and allocate a temp file that large
            // (if not present or > 100 MB, just stream the data)
            let response = Request::get(Uri::from_str(url.clone().as_str()).unwrap())
                .redirect_policy(RedirectPolicy::Follow)
                .body(())
                .unwrap()
                .send_async()
                .await;

            let Ok(response) = response else {
                return;
            };

            let mut response = response.into_body();

            loop {
                let mut buf = vec![0; 2048];
                let Ok(result) = response.read(&mut buf).await else {
                    break;
                };
                debug!("Read {} bytes", result);
                buf.truncate(result);
                if rb.slots_free() >= buf.len() {
                    producer.write(&buf).unwrap();
                } else {
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
            Some(bytes_read) => Ok(bytes_read),
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
