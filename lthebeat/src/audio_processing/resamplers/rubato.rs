use crate::audio_processing::audio_pipeline::PipelineSample;
use crate::audio_processing::audio_pipeline::audio_format::{AudioFormat, SampleFormat};
use crate::audio_processing::audio_pipeline::faucet::{Faucet, FaucetError, create_faucet};
use crate::audio_processing::audio_pipeline::sink::{
    ResetListenerGroup, ResetListenerGroupTrait, Sink, create_sink,
};
use crate::audio_processing::sample::{Sample, SampleData};
use async_ringbuf::traits::AsyncProducer;
use rubato::{FftFixedIn, Resampler};
use smol::stream::StreamExt;
use tracing::info;

pub struct RubatoResampler {
    sink: Option<Sink>,
    faucet: Option<Faucet>,
}

impl RubatoResampler {
    pub fn new(target_audio_format: AudioFormat) -> Self {
        let (faucet, mut rb_faucet_prod, reset_faucet) = create_faucet();
        let (sink, mut rb_sink_cons) = create_sink();

        let reset_listeners = sink.reset_listeners();
        reset_listeners.add_reset_listener(reset_faucet);

        smol::spawn(async move {
            let mut resampler = RubatoResamplerWrapper::new(target_audio_format);
            let mut sample_buffer = Vec::new();
            let mut ct = reset_listeners.create_cancellation_token();
            loop {
                if ct.is_canceled() {
                    sample_buffer.clear();
                    ct = reset_listeners.create_cancellation_token();
                }
                match rb_sink_cons.next().await {
                    Some(Ok(PipelineSample::Sample(next_sample))) => {
                        if matches!(next_sample.data, SampleData::Empty) {
                            // Propagate the empty sample
                            rb_faucet_prod
                                .push(Ok(PipelineSample::Sample(next_sample)))
                                .await
                                .expect("failed to push sample to sink");

                            continue;
                        }

                        if ct.is_canceled() {
                            continue;
                        }

                        let channels = next_sample.channels;
                        if resampler
                            .reconfigure_if_required(next_sample.sample_rate, next_sample.channels)
                        {
                            // The resampler had to be reconfigured
                            sample_buffer.clear();
                        }
                        let meta = next_sample.meta.clone();
                        let sample_id = next_sample.sample_id;
                        let elapsed = next_sample.elapsed_since_start;
                        let associated_track = next_sample.associated_track.clone();
                        if resampler.is_resampling_required() {
                            let resampler = &mut resampler.resampler;
                            sample_buffer.append(&mut next_sample.into_f32());

                            let required_samples =
                                resampler.input_frames_next() * channels as usize;

                            while required_samples <= sample_buffer.len() {
                                if ct.is_canceled() {
                                    continue;
                                }

                                // Deinterleave samples
                                let mut processed_samples = Vec::new();
                                processed_samples.resize(channels as usize, Vec::new());

                                for i in 0..required_samples {
                                    processed_samples[i % channels as usize].push(sample_buffer[i]);
                                }
                                sample_buffer = sample_buffer[required_samples..].to_vec();

                                // Rudimentary up/downmixing
                                if channels == 1 && target_audio_format.channels != 1 {
                                    // Special case: mono to n channels
                                    for _ in 0..target_audio_format.channels - 1 {
                                        processed_samples.push(processed_samples[0].clone());
                                    }
                                } else if target_audio_format.channels < channels {
                                    processed_samples
                                        .truncate(target_audio_format.channels as usize);
                                }

                                // Resample!
                                let resample_result =
                                    resampler.process(&processed_samples, None).unwrap();

                                // Interleave samples
                                let mut resampled_buffer = Vec::new();
                                for i in 0..resample_result.first().unwrap().len() {
                                    for channel_samples in &resample_result {
                                        resampled_buffer.push(channel_samples[i]);
                                    }
                                }

                                // Build sample object
                                let next_sample = Sample::new(
                                    target_audio_format.sample_rate,
                                    target_audio_format.channels,
                                    meta.clone(),
                                    Some(sample_id),
                                    elapsed,
                                    match target_audio_format.sample {
                                        SampleFormat::Signed8 => SampleData::Signed8(Vec::new()),
                                        SampleFormat::Unsigned8 => {
                                            SampleData::Unsigned8(Vec::new())
                                        }
                                        SampleFormat::Unsigned16 => {
                                            SampleData::Unsigned16(Vec::new())
                                        }
                                        SampleFormat::Signed16 => SampleData::Signed16(Vec::new()),
                                        SampleFormat::Unsigned24 => {
                                            SampleData::Unsigned24(Vec::new())
                                        }
                                        SampleFormat::Signed24 => SampleData::Signed24(Vec::new()),
                                        SampleFormat::Unsigned32 => {
                                            SampleData::Unsigned32(Vec::new())
                                        }
                                        SampleFormat::Signed32 => SampleData::Signed32(Vec::new()),
                                        SampleFormat::Unsigned64 => {
                                            SampleData::Unsigned64(Vec::new())
                                        }
                                        SampleFormat::Signed64 => SampleData::Signed64(Vec::new()),
                                        SampleFormat::Float32 => SampleData::Float32(Vec::new()),
                                        SampleFormat::Float64 => SampleData::Float64(Vec::new()),
                                    },
                                    associated_track.clone(),
                                );
                                let next_sample = next_sample.convert_from_f64(resampled_buffer);

                                if ct.is_canceled() {
                                    continue;
                                }

                                rb_faucet_prod
                                    .push(Ok(PipelineSample::Sample(next_sample)))
                                    .await
                                    .expect("failed to push sample to sink");
                            }
                        } else if SampleFormat::from(next_sample.clone())
                            != target_audio_format.sample
                        {
                            let f32_samples = next_sample.into_f32();
                            let next_sample = Sample::new(
                                target_audio_format.sample_rate,
                                target_audio_format.channels,
                                meta,
                                Some(sample_id),
                                elapsed,
                                match target_audio_format.sample {
                                    SampleFormat::Signed8 => SampleData::Signed8(Vec::new()),
                                    SampleFormat::Unsigned8 => SampleData::Unsigned8(Vec::new()),
                                    SampleFormat::Unsigned16 => SampleData::Unsigned16(Vec::new()),
                                    SampleFormat::Signed16 => SampleData::Signed16(Vec::new()),
                                    SampleFormat::Unsigned24 => SampleData::Unsigned24(Vec::new()),
                                    SampleFormat::Signed24 => SampleData::Signed24(Vec::new()),
                                    SampleFormat::Unsigned32 => SampleData::Unsigned32(Vec::new()),
                                    SampleFormat::Signed32 => SampleData::Signed32(Vec::new()),
                                    SampleFormat::Unsigned64 => SampleData::Unsigned64(Vec::new()),
                                    SampleFormat::Signed64 => SampleData::Signed64(Vec::new()),
                                    SampleFormat::Float32 => SampleData::Float32(Vec::new()),
                                    SampleFormat::Float64 => SampleData::Float64(Vec::new()),
                                },
                                associated_track,
                            );
                            let next_sample = next_sample.convert_from_f64(f32_samples);
                            rb_faucet_prod
                                .push(Ok(PipelineSample::Sample(next_sample)))
                                .await
                                .expect("failed to push sample to sink");
                        } else {
                            rb_faucet_prod
                                .push(Ok(PipelineSample::Sample(next_sample)))
                                .await
                                .expect("failed to push sample to sink");
                        }
                    }
                    Some(Err(err)) => {
                        // Propagate the sample
                        rb_faucet_prod
                            .push(Err(err))
                            .await
                            .expect("failed to push sample to sink");
                    }
                    None => {
                        rb_faucet_prod
                            .push(Err(FaucetError::UnknownError))
                            .await
                            .expect("failed to push sample to sink");
                    }
                };
            }
        })
        .detach();

        RubatoResampler {
            sink: Some(sink),
            faucet: Some(faucet),
        }
    }

    pub fn sink(&mut self) -> Sink {
        self.sink
            .take()
            .expect("RubatoResampler: tried to take sink twice")
    }

    pub fn faucet(&mut self) -> Faucet {
        self.faucet
            .take()
            .expect("RubatoResampler: tried to take faucet twice")
    }
}

struct RubatoResamplerWrapper {
    pub resampler: FftFixedIn<f64>,
    input_sample_rate: u32,
    input_channels: u16,
    output_format: AudioFormat,
}

impl RubatoResamplerWrapper {
    fn new(output_format: AudioFormat) -> RubatoResamplerWrapper {
        RubatoResamplerWrapper {
            resampler: FftFixedIn::<f64>::new(
                44100,
                output_format.sample_rate as usize,
                1024,
                2,
                output_format.channels as usize,
            )
            .unwrap(),
            input_sample_rate: 44100,
            input_channels: 2,
            output_format,
        }
    }

    fn reconfigure_if_required(&mut self, input_sample_rate: u32, input_channels: u16) -> bool {
        if self.input_sample_rate != input_sample_rate {
            info!("RubatoResampler: reconfiguring resampler to new input format");
            info!(
                "RubatoResampler: input sample rate: {}, input channels: {}",
                input_sample_rate, input_channels
            );
            info!(
                "RubatoResampler: output sample rate: {}, output channels: {}",
                self.output_format.sample_rate, self.output_format.channels
            );
            self.resampler = FftFixedIn::<f64>::new(
                input_sample_rate as usize,
                self.output_format.sample_rate as usize,
                1024,
                2,
                self.output_format.channels as usize,
            )
            .unwrap();
            self.input_sample_rate = input_sample_rate;
            self.input_channels = input_channels;
            true
        } else {
            false
        }
    }

    fn is_resampling_required(&self) -> bool {
        self.input_sample_rate != self.output_format.sample_rate
            || self.input_channels != self.output_format.channels
    }
}
