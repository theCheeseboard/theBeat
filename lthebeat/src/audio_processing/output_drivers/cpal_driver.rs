use crate::audio_processing::audio_pipeline::audio_format::AudioFormat;
use crate::audio_processing::audio_pipeline::{SAMPLE_BUFFER_SIZE, plug, PipelineSampleResult, PipelineSample};
use crate::audio_processing::mute::Mute;
use crate::audio_processing::output_drivers::{OutputDevice, Sample, Sink};
use crate::audio_processing::resamplers::rubato::RubatoResampler;
use crate::audio_processing::sample::UnwrapSample;
use async_ringbuf::AsyncHeapRb;
use async_ringbuf::traits::{AsyncProducer, Based, Consumer, Split};
use cpal::traits::{DeviceTrait, HostTrait, StreamTrait};
use cpal::{Device, SampleFormat, SizedSample, Stream, StreamConfig};
use gpui::http_client::anyhow;
use gpui::private::anyhow;
use log::warn;
use smol::stream::StreamExt;
use std::cell::RefCell;
use std::time::Duration;
use tracing::{error, info};
use crate::audio_processing::audio_pipeline::sink::create_sink;

const BUFFER_DURATION: Duration = Duration::from_millis(200);
const BUFFER_DURATION_MSEC: usize = BUFFER_DURATION.as_millis() as usize;

struct CpalOutputDevice {
    device: Device,
    streams: RefCell<Vec<Stream>>,
}

impl CpalOutputDevice {
    pub fn new(device: Device) -> Self {
        Self {
            device,
            streams: RefCell::new(Vec::new()),
        }
    }

    fn create_stream<T>(&self, config: StreamConfig) -> anyhow::Result<Sink>
    where
        T: Clone + Copy + CpalSample + Sync,
        Sample: UnwrapSample<Vec<T>>,
    {
        let buffer_size = ((BUFFER_DURATION_MSEC * config.sample_rate.0 as usize) / 1000)
            * config.channels as usize;

        let (mut producer, mut consumer) = AsyncHeapRb::<T>::new(buffer_size).split();

        let mut was_underrun = false;
        let stream = self.device.build_output_stream(
            &config,
            move |data: &mut [T], _: &cpal::OutputCallbackInfo| {
                let written = consumer.pop_slice(data);

                if written < data.len() {
                    if !was_underrun {
                        warn!("CpalOutputDevice: buffer underrun");
                        was_underrun = true;
                    }
                    data[written..].iter_mut().for_each(|v| *v = T::muted())
                } else {
                    was_underrun = false;
                }
            },
            move |err| {
                // Errors? What errors!?
                error!("CpalOutputDevice: error in output stream: {:?}", err)
            },
            None,
        )?;

        let (sink, mut samples_consumer) = create_sink();
        smol::spawn(async move {
            loop {
                let samples = samples_consumer.next().await;
                match samples {
                    Some(Ok(PipelineSample::Sample(samples))) => {
                        let samples_vec = samples.unwrap();
                        producer.push_exact(samples_vec).await.unwrap();
                    }
                    Some(Ok(PipelineSample::Reset)) => {
                        // Clear the pipeline

                    }
                    Some(Err(err)) => {
                        info!("CpalOutputDevice: error in samples producer: {:?}", err);
                        return;
                    }
                    None => {
                        info!("CpalOutputDevice: sink source closed");
                        return;
                    }
                }
            }
        })
        .detach();

        self.streams.borrow_mut().push(stream);

        Ok(sink)
    }
}

impl OutputDevice for CpalOutputDevice {
    fn pause(&self) {
        self.streams
            .borrow()
            .iter()
            .for_each(|stream| stream.pause().unwrap());
    }

    fn play(&self) {
        self.streams
            .borrow()
            .iter()
            .for_each(|stream| stream.play().unwrap());
    }

    fn open_sink(&self) -> anyhow::Result<Sink> {
        let supported_stream_config = self.device.default_output_config().unwrap();
        let sample_format = supported_stream_config.sample_format();
        let config = supported_stream_config.config();

        let mut resampler = RubatoResampler::new(AudioFormat {
            sample_rate: config.sample_rate.0,
            channels: config.channels,
            sample: sample_format.into(),
        });

        let audio_device_sink = match sample_format {
            SampleFormat::I8 => self.create_stream::<i8>(config),
            SampleFormat::I16 => self.create_stream::<i16>(config),
            SampleFormat::I24 => Err(anyhow!(
                "CpalOutputDevice::open_sink: unsupported sample format i24"
            )),
            SampleFormat::I32 => self.create_stream::<i32>(config),
            SampleFormat::I64 => Err(anyhow!(
                "CpalOutputDevice::open_sink: unsupported sample format i64"
            )),
            SampleFormat::U8 => self.create_stream::<u8>(config),
            SampleFormat::U16 => self.create_stream::<u16>(config),
            SampleFormat::U32 => self.create_stream::<u32>(config),
            SampleFormat::U64 => Err(anyhow!(
                "CpalOutputDevice::open_sink: unsupported sample format u64"
            )),
            SampleFormat::F32 => self.create_stream::<f32>(config),
            SampleFormat::F64 => self.create_stream::<f64>(config),
            _ => Err(anyhow!(
                "CpalOutputDevice::open_sink: unsupported sample format"
            )),
        }?;

        plug(resampler.faucet(), audio_device_sink);

        Ok(resampler.sink())
    }
}

trait CpalSample: SizedSample + Default + Send + Sized + 'static + Mute {}

impl<T> CpalSample for T where T: SizedSample + Default + Send + Sized + 'static + Mute {}

pub fn cpal_output_devices() -> Vec<Box<dyn OutputDevice>> {
    let Ok(output_devices) = cpal::default_host().output_devices() else {
        return Vec::new();
    };

    output_devices
        .map(|output_device| {
            Box::new(CpalOutputDevice::new(output_device)) as Box<dyn OutputDevice>
        })
        .collect::<Vec<_>>()
}

pub fn cpal_default_output_device() -> Box<dyn OutputDevice> {
    Box::new(CpalOutputDevice::new(
        cpal::default_host().default_output_device().unwrap(),
    ))
}

impl From<SampleFormat> for crate::audio_processing::audio_pipeline::audio_format::SampleFormat {
    fn from(value: SampleFormat) -> Self {
        match value {
            SampleFormat::I8 => {
                crate::audio_processing::audio_pipeline::audio_format::SampleFormat::Signed8
            }
            SampleFormat::I16 => {
                crate::audio_processing::audio_pipeline::audio_format::SampleFormat::Signed16
            }
            SampleFormat::I24 => {
                crate::audio_processing::audio_pipeline::audio_format::SampleFormat::Signed24
            }
            SampleFormat::I32 => {
                crate::audio_processing::audio_pipeline::audio_format::SampleFormat::Signed32
            }
            SampleFormat::I64 => {
                crate::audio_processing::audio_pipeline::audio_format::SampleFormat::Signed64
            }
            SampleFormat::U8 => {
                crate::audio_processing::audio_pipeline::audio_format::SampleFormat::Unsigned8
            }
            SampleFormat::U16 => {
                crate::audio_processing::audio_pipeline::audio_format::SampleFormat::Unsigned16
            }
            SampleFormat::U32 => {
                crate::audio_processing::audio_pipeline::audio_format::SampleFormat::Unsigned32
            }
            SampleFormat::U64 => {
                crate::audio_processing::audio_pipeline::audio_format::SampleFormat::Unsigned64
            }
            SampleFormat::F32 => {
                crate::audio_processing::audio_pipeline::audio_format::SampleFormat::Float32
            }
            SampleFormat::F64 => {
                crate::audio_processing::audio_pipeline::audio_format::SampleFormat::Float64
            }
            _ => panic!("Unknown sample format"),
        }
    }
}
