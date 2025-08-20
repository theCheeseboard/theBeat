use crate::audio_processing::audio_pipeline::sink::Sink;
use crate::audio_processing::sample::Sample;

pub mod cpal_driver;

pub trait OutputDevice {
    fn pause(&self);

    fn play(&self);
}
