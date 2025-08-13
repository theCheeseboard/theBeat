use crate::audio_processing::audio_pipeline::faucet::Faucet;
use crate::audio_processing::audio_pipeline::plug;
use crate::audio_processing::audio_pipeline::sink::Sink;
use crate::audio_processing::input_engines::faucet_for_url;
use crate::audio_processing::output_drivers::cpal_driver::cpal_output_devices;
use crate::audio_processing::sample::Sample;
use gpui::{App, Global};
use std::str::FromStr;
use std::sync::Arc;
use url::Url;

pub struct AudioController {}

enum SinkInMessage {
    Sample(Sample),
}

enum SinkOutMessage {
    SampleReturn(Sample),
}

impl AudioController {
    pub fn new() -> Arc<AudioController> {
        let audio_controller = Arc::new(AudioController {});
        let controller = audio_controller.clone();

        audio_controller
    }

    pub fn play_url(&self, url: Url) {
        let device = cpal_output_devices().into_iter().next().unwrap();
        let sink = device.open_sink().unwrap();

        let engine = faucet_for_url(url).unwrap();

        plug(engine, sink);

        device.play();
        Box::leak(device);
    }

    pub fn play() {}

    pub fn stop() {}

    pub fn enqueue() {}

    pub fn default_audio_device() {}
}

pub struct GlobalAudioController {
    pub audio_controller: Arc<AudioController>,
}

impl GlobalAudioController {
    pub fn new(audio_controller: Arc<AudioController>) -> GlobalAudioController {
        GlobalAudioController { audio_controller }
    }
}

impl Global for GlobalAudioController {}
