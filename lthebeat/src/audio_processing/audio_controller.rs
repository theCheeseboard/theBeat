use crate::audio_processing::audio_metadata::AudioMetadata;
use crate::audio_processing::audio_pipeline::attenuator::Attenuator;
use crate::audio_processing::audio_pipeline::duplicator::Duplicator;
use crate::audio_processing::audio_pipeline::plug;
use crate::audio_processing::audio_pipeline::sink::Sink;
use crate::audio_processing::audio_pipeline::sync_lock_sync::SyncLockSync;
use crate::audio_processing::output_drivers::OutputDevice;
use crate::audio_processing::output_drivers::cpal_driver::cpal_default_output_device;
use crate::platform::{Platform, PlatformHandler};
use crate::play_queue::media_item::MediaItem;
use cntp_i18n::{I18N_MANAGER, tr_load};
use gpui::{App, AppContext, AsyncApp, BorrowAppContext, Entity, Global};
use std::time::Duration;

pub struct AudioController {
    pub sync_lock_sync: SyncLockSync,
    is_playing: bool,
    duplicator: Duplicator,
    connected_devices: Vec<Box<dyn OutputDevice>>,
    master_volume: f64,
}

impl AudioController {
    pub fn new(cx: &mut App) -> AudioController {
        let mut sync_lock_sync = SyncLockSync::new();
        let event_channel = sync_lock_sync.event_channel();
        let current_meta = sync_lock_sync.current_meta.clone();

        let duplicator = Duplicator::new();

        cx.spawn(async move |cx: &mut AsyncApp| {
            loop {
                I18N_MANAGER.write().unwrap().load_source(tr_load!());
                event_channel.recv().await.unwrap();
                cx.update_global::<AudioController, ()>(|_, _| {
                    // Do nothing
                });
                cx.update_global::<Platform, ()>(|platform, cx| {
                    let metadata = current_meta.read().unwrap().clone();
                    platform.new_metadata_available(metadata, cx)
                });
                cx.refresh();
            }
        })
        .detach();

        let mut audio_controller = AudioController {
            sync_lock_sync,
            is_playing: true,
            duplicator,
            connected_devices: Vec::new(),
            master_volume: 1.0,
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

        let factor = logarithmic_attenuation_factor(self.master_volume);
        device.set_attenuation_factor(factor);

        plug(self.duplicator.open_faucet(), sink.sink);
        self.sync_lock_sync.manage(sink.sync_lock);

        if self.is_playing {
            device.play();
        } else {
            device.pause();
        }
        self.connected_devices.push(device);
    }

    pub fn play(&mut self, cx: &mut App) {
        self.is_playing = true;
        for device in self.connected_devices.iter_mut() {
            device.play();
        }

        cx.update_global::<Platform, ()>(|platform, cx| {
            platform.play_state_changed(true, cx);
        });
    }

    pub fn pause(&mut self, cx: &mut App) {
        self.is_playing = false;
        for device in self.connected_devices.iter_mut() {
            device.pause();
        }

        cx.update_global::<Platform, ()>(|platform, cx| {
            platform.play_state_changed(false, cx);
        });
    }

    pub fn is_playing(&self) -> bool {
        self.is_playing
    }

    pub fn play_pause(&mut self, cx: &mut App) {
        if self.is_playing {
            self.pause(cx);
        } else {
            self.play(cx);
        }
    }

    pub fn current_metadata(&self) -> AudioMetadata {
        self.sync_lock_sync.current_meta.read().unwrap().clone()
    }

    pub fn current_time(&self) -> Option<Duration> {
        *self.sync_lock_sync.current_time.read().unwrap()
    }

    pub fn current_track(&self) -> Option<Entity<MediaItem>> {
        self.sync_lock_sync.current_track.read().unwrap().clone()
    }

    pub fn sink(&mut self) -> Sink {
        self.duplicator.sink()
    }

    pub fn set_master_volume(&mut self, volume: f64) {
        let volume = volume.clamp(0., 1.);
        self.master_volume = volume;

        let factor = logarithmic_attenuation_factor(volume);
        for device in self.connected_devices.iter_mut() {
            device.set_attenuation_factor(factor);
        }
    }

    pub fn master_volume(&self) -> f64 {
        self.master_volume
    }
}

impl Global for AudioController {}

fn logarithmic_attenuation_factor(volume: f64) -> f64 {
    if volume > 0.99 {
        1.
    } else if volume < 0.01 {
        0.
    } else {
        -(1. - volume).log(100.)
    }
}
