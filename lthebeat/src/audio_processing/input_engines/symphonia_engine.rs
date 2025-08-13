mod http_source;

use tracing::info;
use crate::audio_processing::audio_pipeline::faucet::{Faucet, FaucetError};
use crate::audio_processing::audio_pipeline::{PipelineSample, SAMPLE_BUFFER_SIZE};
use crate::audio_processing::input_engines::symphonia_engine::http_source::HttpSource;
use crate::audio_processing::sample::{Sample, SampleData};
use async_ringbuf::AsyncHeapRb;
use async_ringbuf::traits::{AsyncProducer, Consumer, Split};
use isahc::RequestExt;
use isahc::config::Configurable;
use log::warn;
use rb::RB;
use smol::io::AsyncWriteExt;
use smol::stream::StreamExt;
use std::fs::File;
use std::thread;
use symphonia::core::audio::{AudioBufferRef, Signal};
use symphonia::core::codecs::Decoder;
use symphonia::core::io::{MediaSource, MediaSourceStream, MediaSourceStreamOptions};
use symphonia::core::meta::MetadataRevision;
use symphonia::core::probe::Hint;
use symphonia::default::{get_codecs, get_probe};
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
                Box::new(HttpSource::new(url)) as Box<dyn MediaSource>,
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

        let (mut rb_prod, rb_cons) = AsyncHeapRb::<PipelineSample>::new(SAMPLE_BUFFER_SIZE).split();
        thread::spawn(move || {
            loop {
                let next_sample = {
                    let next_packet = match format.next_packet() {
                        Ok(packet) => packet,
                        Err(err) => {
                            warn!("SymphoniaEngine: error while reading packet: {}", err);
                            smol::block_on(rb_prod.push(Err(FaucetError::UnknownError))).unwrap();
                            return;
                        }
                    };

                    let decoded = decoder.decode(&next_packet).unwrap();

                    let rate = decoded.spec().rate;
                    let channel_count = decoded.spec().channels.count();

                    let sample_data = match decoded {
                        AudioBufferRef::U8(v) => {
                            let mut samples: Vec<u8> =
                                Vec::with_capacity(v.spec().channels.count() * v.capacity());

                            for i in 0..channel_count {
                                for sample in v.chan(i) {
                                    samples[i * channel_count + i] = *sample;
                                }
                            }

                            SampleData::Unsigned8(samples)
                        }
                        AudioBufferRef::U16(v) => {
                            let mut samples: Vec<u16> =
                                Vec::with_capacity(v.spec().channels.count() * v.capacity());

                            for i in 0..channel_count {
                                for sample in v.chan(i) {
                                    samples[i * channel_count + i] = *sample;
                                }
                            }

                            SampleData::Unsigned16(samples)
                        }
                        AudioBufferRef::U32(v) => {
                            let mut samples: Vec<u32> =
                                Vec::with_capacity(v.spec().channels.count() * v.capacity());

                            for i in 0..channel_count {
                                for sample in v.chan(i) {
                                    samples[i * channel_count + i] = *sample;
                                }
                            }

                            SampleData::Unsigned32(samples)
                        }
                        AudioBufferRef::S8(v) => {
                            let mut samples: Vec<i8> =
                                Vec::with_capacity(v.spec().channels.count() * v.capacity());

                            for i in 0..channel_count {
                                for sample in v.chan(i) {
                                    samples[i * channel_count + i] = *sample;
                                }
                            }

                            SampleData::Signed8(samples)
                        }
                        AudioBufferRef::S16(v) => {
                            let mut samples: Vec<i16> =
                                Vec::with_capacity(v.spec().channels.count() * v.capacity());

                            for i in 0..channel_count {
                                for sample in v.chan(i) {
                                    samples[i * channel_count + i] = *sample;
                                }
                            }

                            SampleData::Signed16(samples)
                        }
                        AudioBufferRef::S32(v) => {
                            let mut samples: Vec<i32> =
                                Vec::with_capacity(v.spec().channels.count() * v.capacity());
                            samples.resize(v.spec().channels.count() * v.capacity(), 0);

                            for i in 0..channel_count {
                                let chan = v.chan(i);
                                for sample_i in 0..chan.len() {
                                    samples[sample_i * channel_count + i] = chan[sample_i];
                                }
                            }

                            SampleData::Signed32(samples)
                        }
                        AudioBufferRef::F32(v) => {
                            let mut samples: Vec<f32> =
                                Vec::with_capacity(v.spec().channels.count() * v.capacity());
                            samples.resize(v.spec().channels.count() * v.capacity(), 0.);

                            for i in 0..channel_count {
                                let chan = v.chan(i);
                                for sample_i in 0..chan.len() {
                                    samples[sample_i * channel_count + i] = chan[sample_i];
                                }
                            }

                            SampleData::Float32(samples)
                        }
                        AudioBufferRef::F64(v) => {
                            let mut samples: Vec<f64> =
                                Vec::with_capacity(v.spec().channels.count() * v.capacity());
                            samples.resize(v.spec().channels.count() * v.capacity(), 0.);

                            for i in 0..channel_count {
                                let chan = v.chan(i);
                                for sample_i in 0..chan.len() {
                                    samples[sample_i * channel_count + i] = chan[sample_i];
                                }
                            }

                            SampleData::Float64(samples)
                        }
                        _ => panic!("Panic"),
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

                    Some(Sample::new(rate, channel_count as u16, sample_data))
                };

                if next_sample.is_none() {
                    smol::block_on(rb_prod.push(Err(FaucetError::EndOfStream))).unwrap();
                    return;
                }

                smol::block_on(rb_prod.push(next_sample.ok_or(FaucetError::UnknownError))).unwrap();
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
