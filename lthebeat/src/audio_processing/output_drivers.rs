use std::vec;
use cpal::U24;
use gpui::private::anyhow;
use intx::I24;
use crate::audio_processing::audio_pipeline::sink::Sink;
use crate::audio_processing::sample::Sample;

pub mod cpal_driver;

pub trait OutputDevice {
    fn pause(&self);

    fn play(&self);

    fn open_sink(&self) -> anyhow::Result<Sink>;
}
