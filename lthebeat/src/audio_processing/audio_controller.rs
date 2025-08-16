use crate::audio_processing::output_drivers::cpal_driver::{cpal_default_output_device, cpal_output_devices};
use crate::audio_processing::{audio_pipeline::plug, input_engines::faucet_for_url};
use gpui::Global;
use std::sync::Arc;
use url::Url;
use crate::audio_processing::audio_pipeline::duplicator::Duplicator;

pub struct AudioController {}

impl AudioController {
    pub fn new() -> Arc<AudioController> {
        Arc::new(AudioController {})
    }

    pub fn play_url(&self, url: Url) {
        let engine = faucet_for_url(url).unwrap();
        let mut duplicator = Duplicator::new();

        for device in cpal_output_devices() {
            let sink = device.open_sink().unwrap();

            plug(duplicator.open_faucet(), sink);

            device.play();
            Box::leak(device);
        }

        plug(engine, duplicator.sink());
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
