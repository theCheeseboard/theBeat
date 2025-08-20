mod http_source;

use crate::audio_processing::audio_metadata::AudioMetadata;
use crate::audio_processing::audio_pipeline::faucet::{Faucet, FaucetError};
use crate::audio_processing::audio_pipeline::{
    PipelineSample, PipelineSampleResult, SAMPLE_BUFFER_SIZE,
};
use crate::audio_processing::input_engines::symphonia_engine::http_source::HttpSource;
use crate::audio_processing::sample::{Sample, SampleData};
use async_ringbuf::AsyncHeapRb;
use async_ringbuf::traits::{AsyncProducer, Split};
use log::warn;
use std::borrow::Cow;
use std::fs::File;
use std::thread;
use symphonia::core::audio::{AudioBuffer, AudioBufferRef, Signal};
use symphonia::core::io::{MediaSource, MediaSourceStream, MediaSourceStreamOptions};
use symphonia::core::meta::{MetadataRevision, StandardTagKey};
use symphonia::core::probe::Hint;
use symphonia::default::{get_codecs, get_probe};
use tracing::info;
use url::Url;

pub struct SymphoniaEngine {
    faucet: Option<Faucet>,
}

impl SymphoniaEngine {
    pub fn new(url: Url) -> anyhow::Result<Self> {
        let (music, hint) = match url.scheme() {
            "file" => {
                let path = url
                    .to_file_path()
                    .map_err(|_| anyhow::anyhow!("Unable to decode file path from URL"))?;
                let mut hint = Hint::new();
                hint.with_extension(path.extension().unwrap().to_str().unwrap());
                (Box::new(File::open(path)?) as Box<dyn MediaSource>, hint)
            }
            "http" | "https" => (
                Box::new(HttpSource::new(url.clone())) as Box<dyn MediaSource>,
                Hint::new(),
            ),
            _ => return Err(anyhow::anyhow!("Unsupported scheme")),
        };

        let media_source_stream =
            MediaSourceStream::new(music, MediaSourceStreamOptions::default());
        let meta_opts = Default::default();
        let format_opts = Default::default();
        let probe = get_probe();
        let mut probe_result =
            probe.format(&hint, media_source_stream, &format_opts, &meta_opts)?;

        let codec_registry = get_codecs();
        let mut format = probe_result.format;
        let first_track = format.default_track().unwrap();
        let decoder_opts = Default::default();
        let mut decoder = codec_registry.make(&first_track.codec_params, &decoder_opts)?;

        let (mut rb_prod, rb_cons) =
            AsyncHeapRb::<PipelineSampleResult>::new(SAMPLE_BUFFER_SIZE).split();
        thread::spawn(move || {
            let mut file_meta = AudioMetadata {
                url: Some(url),
                ..AudioMetadata::default()
            };
            if let Some(probe_meta) = probe_result
                .metadata
                .get()
                .as_ref()
                .and_then(|m| m.current())
            {
                populate_metadata(&mut file_meta, probe_meta);
            }
            if let Some(next_meta) = format.metadata().current() {
                populate_metadata(&mut file_meta, next_meta);
            }

            let mut pushed_first_sample = false;
            loop {
                let next_sample = {
                    let next_packet = match format.next_packet() {
                        Ok(packet) => packet,
                        Err(err) => {
                            warn!("SymphoniaEngine: error while reading packet: {err}");
                            smol::block_on(rb_prod.push(Err(FaucetError::UnknownError))).unwrap();
                            return;
                        }
                    };

                    while !format.metadata().is_latest() {
                        format.metadata().pop();
                        if let Some(next_meta) = format.metadata().current() {
                            populate_metadata(&mut file_meta, next_meta);
                        }
                    }

                    let decoded = decoder.decode(&next_packet).unwrap();

                    let rate = decoded.spec().rate;
                    let channel_count = decoded.spec().channels.count();

                    let sample_data = match decoded {
                        AudioBufferRef::U8(v) => SampleData::Unsigned8(create_sample_data(v)),
                        AudioBufferRef::U16(v) => SampleData::Unsigned16(create_sample_data(v)),
                        AudioBufferRef::U32(v) => SampleData::Unsigned32(create_sample_data(v)),
                        AudioBufferRef::S8(v) => SampleData::Signed8(create_sample_data(v)),
                        AudioBufferRef::S16(v) => SampleData::Signed16(create_sample_data(v)),
                        AudioBufferRef::S32(v) => SampleData::Signed32(create_sample_data(v)),
                        AudioBufferRef::F32(v) => SampleData::Float32(create_sample_data(v)),
                        AudioBufferRef::F64(v) => SampleData::Float64(create_sample_data(v)),
                        _ => panic!("SymphoniaEngine: unsupported sample format"),
                    };

                    let mut metadata = format.metadata();
                    if !metadata.is_latest() {
                        match metadata.skip_to_latest() {
                            None => {}
                            Some(meta) => {
                                for tag in meta.tags() {
                                    info!("tag: {}, {}", tag.key, tag.value.to_string())
                                }
                            }
                        }
                    }

                    if !pushed_first_sample {
                        if smol::block_on(rb_prod.push(Ok(PipelineSample::Sample(Sample::new(
                            rate,
                            channel_count as u16,
                            file_meta.clone(),
                            None,
                            SampleData::Empty,
                        )))))
                            .is_err()
                        {
                            warn!("SymphoniaEngine: error while pushing sample to buffer");
                            warn!("SymphoniaEngine: stopping");
                            return;
                        }
                        pushed_first_sample = true
                    }

                    Some(Sample::new(
                        rate,
                        channel_count as u16,
                        file_meta.clone(),
                        None,
                        sample_data,
                    ))
                };

                if next_sample.is_none() {
                    smol::block_on(rb_prod.push(Err(FaucetError::EndOfStream))).unwrap();
                    return;
                }

                if smol::block_on(
                    rb_prod.push(
                        next_sample
                            .ok_or(FaucetError::UnknownError)
                            .map(PipelineSample::Sample),
                    ),
                )
                .is_err()
                {
                    warn!("SymphoniaEngine: error while pushing sample to buffer");
                    warn!("SymphoniaEngine: stopping");
                    return;
                }
            }
        });

        Ok(Self {
            faucet: Some(Faucet::new(rb_cons)),
        })
    }

    pub fn faucet(&mut self) -> Faucet {
        self.faucet
            .take()
            .expect("SymphoniaEngine: tried to take faucet twice")
    }
}

fn create_sample_data<T>(v: Cow<AudioBuffer<T>>) -> Vec<T>
where
    T: symphonia::core::sample::Sample,
{
    let channel_count = v.spec().channels.count();
    let mut samples: Vec<T> = vec![T::default(); channel_count * v.capacity()];

    for i in 0..channel_count {
        let chan = v.chan(i);
        for sample_i in 0..chan.len() {
            samples[sample_i * channel_count + i] = chan[sample_i];
        }
    }

    samples
}

fn populate_metadata(metadata: &mut AudioMetadata, symphonia_metadata: &MetadataRevision) {
    for tag in symphonia_metadata.tags() {
        match tag.std_key {
            Some(StandardTagKey::TrackTitle) => metadata.title = Some(tag.value.to_string()),
            Some(StandardTagKey::Artist) => metadata.artist = Some(tag.value.to_string()),
            Some(StandardTagKey::Album) => metadata.album = Some(tag.value.to_string()),
            _ => {}
        }
    }
}
