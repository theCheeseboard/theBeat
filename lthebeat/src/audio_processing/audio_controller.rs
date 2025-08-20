use crate::audio_processing::audio_metadata::AudioMetadata;
use crate::audio_processing::audio_pipeline::sync_lock_sync::SyncLockSync;
use gpui::Global;

pub struct AudioController {
    pub sync_lock_sync: SyncLockSync
}

impl Default for AudioController {
    fn default() -> Self {
        Self::new()
    }
}

impl AudioController {
    pub fn new() -> AudioController {
        AudioController {
            sync_lock_sync: SyncLockSync::new()
        }
    }

    pub fn play() {}

    pub fn stop() {}

    pub fn enqueue() {}

    pub fn default_audio_device() {}

    pub fn current_metadata(&self) -> AudioMetadata {
        self.sync_lock_sync.current_meta.read().unwrap().clone()
    }
}

impl Global for AudioController {}
