use crate::audio_processing::audio_metadata::AudioMetadata;
use crate::audio_processing::audio_pipeline::duplicator::Duplicator;
use crate::audio_processing::audio_pipeline::plug;
use crate::audio_processing::audio_pipeline::sink::Sink;
use crate::audio_processing::audio_pipeline::sync_lock_sync::SyncLockSync;
use crate::audio_processing::output_drivers::OutputDevice;
use crate::audio_processing::output_drivers::cpal_driver::cpal_default_output_device;
use crate::play_queue::PlayQueue;
use gpui::{App, AsyncApp, Global};
use std::time::Duration;

pub struct AudioController {
    pub sync_lock_sync: SyncLockSync,
    is_playing: bool,
    duplicator: Duplicator,
    connected_devices: Vec<Box<dyn OutputDevice>>,
}

impl AudioController {
    pub fn new(cx: &mut App) -> AudioController {
        let mut sync_lock_sync = SyncLockSync::new();
        let event_channel = sync_lock_sync.event_channel();

        let duplicator = Duplicator::new();

        cx.spawn(async move |cx: &mut AsyncApp| {
            loop {
                event_channel.recv().await.unwrap();
                cx.update_global::<AudioController, ()>(|_, _| {
                    // Do nothing
                })
                    .unwrap();
                cx.refresh().unwrap();
            }
        })
            .detach();

        let mut audio_controller = AudioController {
            sync_lock_sync,
            is_playing: true,
            duplicator,
            connected_devices: Vec::new(),
        };

        let device = cpal_default_output_device();
        audio_controller.connect_audio_device(device);

        audio_controller
    }

    pub fn connect_audio_device(&mut self, device: Box<dyn OutputDevice>) {
        // for device in cpal_output_devices() {
        //     let sink = device.open_sink().unwrap();
        //
        //     plug(play_queue.open_faucet(), sink.sink);
        //     audio_controller.sync_lock_sync.manage(sink.sync_lock);
        //
        //     device.play();
        //     Box::leak(device);
        // }
        let sink = device.open_sink().unwrap();

        plug(self.duplicator.open_faucet(), sink.sink);
        self.sync_lock_sync.manage(sink.sync_lock);

        if self.is_playing {
            device.play();
        } else {
            device.pause();
        }
        self.connected_devices.push(device);
    }

    pub fn play(&mut self) {
        self.is_playing = true;
        for device in self.connected_devices.iter_mut() {
            device.play();
        }
    }

    pub fn pause(&mut self) {
        self.is_playing = false;
        for device in self.connected_devices.iter_mut() {
            device.pause();
        }
    }

    pub fn is_playing(&self) -> bool {
        self.is_playing
    }

    pub fn play_pause(&mut self) {
        if self.is_playing {
            self.pause();
        } else {
            self.play();
        }
    }

    pub fn current_metadata(&self) -> AudioMetadata {
        self.sync_lock_sync.current_meta.read().unwrap().clone()
    }

    pub fn current_time(&self) -> Option<Duration> {
        *self.sync_lock_sync.current_time.read().unwrap()
    }

    pub fn sink(&mut self) -> Sink {
        self.duplicator.sink()
    }
}

impl Global for AudioController {}
