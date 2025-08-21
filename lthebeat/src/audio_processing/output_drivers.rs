use crate::audio_processing::audio_pipeline::audio_format::AudioFormat;
use crate::audio_processing::audio_pipeline::plug;
use crate::audio_processing::audio_pipeline::sink::Sink;
use crate::audio_processing::audio_pipeline::sync_lock::SyncLock;
use crate::audio_processing::resamplers::rubato::RubatoResampler;
use crate::audio_processing::sample::Sample;
use anyhow::anyhow;
use cpal::SampleFormat;
use std::sync::Arc;

pub mod cpal_driver;

pub trait OutputDevice {
    fn pause(&self);

    fn play(&self);

    fn open_sink(&self) -> anyhow::Result<OutputDeviceOutputStream>;
}

pub struct OutputDeviceOutputStream {
    pub sink: Sink,
    pub sync_lock: Arc<SyncLock>,
}
