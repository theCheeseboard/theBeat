use crate::audio_processing::audio_metadata::AudioMetadata;
use crate::audio_processing::audio_pipeline::sync_lock_sync::SyncLockSync;
use gpui::{App, AsyncApp, Global};
use std::time::Duration;

pub struct AudioController {
    pub sync_lock_sync: SyncLockSync,
}

impl AudioController {
    pub fn new(cx: &mut App) -> AudioController {
        let mut sync_lock_sync = SyncLockSync::new();
        let event_channel = sync_lock_sync.event_channel();

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

        AudioController { sync_lock_sync }
    }

    pub fn play() {}

    pub fn stop() {}

    pub fn enqueue() {}

    pub fn default_audio_device() {}

    pub fn current_metadata(&self) -> AudioMetadata {
        self.sync_lock_sync.current_meta.read().unwrap().clone()
    }

    pub fn current_time(&self) -> Option<Duration> {
        *self.sync_lock_sync.current_time.read().unwrap()
    }
}

impl Global for AudioController {}
