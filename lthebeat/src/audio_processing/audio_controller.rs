use crate::audio_processing::audio_pipeline::sink::create_dummy_sink;
use crate::audio_processing::output_drivers::cpal_driver::cpal_default_output_device;
use crate::audio_processing::{audio_pipeline::plug, input_engines::faucet_for_url};
use gpui::Global;
use std::sync::Arc;
use url::Url;

pub struct AudioController {}

impl AudioController {
    pub fn new() -> Arc<AudioController> {
        let audio_controller = Arc::new(AudioController {});
        let controller = audio_controller.clone();

        audio_controller
    }

    pub fn play_url(&self, url: Url) {
        let device = cpal_default_output_device();
        let sink = device.open_sink().unwrap();

        let engine = faucet_for_url(url).unwrap();

        plug(engine, sink);
        // plug(engine, create_dummy_sink());

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
