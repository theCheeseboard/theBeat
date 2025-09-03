use crate::audio_processing::audio_pipeline::ResetPipelineFunction;
use std::sync::{Arc, RwLock};

#[derive(Clone)]
pub struct Resetter {
    reset_listeners: Arc<RwLock<Vec<Arc<RwLock<Vec<ResetPipelineFunction>>>>>>,
    epoch: Arc<RwLock<u16>>,
}

impl Default for Resetter {
    fn default() -> Self {
        Self::new()
    }
}

impl Resetter {
    pub fn new() -> Self {
        Self {
            reset_listeners: Arc::new(RwLock::new(Vec::new())),
            epoch: Arc::new(RwLock::new(0)),
        }
    }

    pub fn trigger_reset(&self) {
        let mut epoch_borrow = self.epoch.write().unwrap();
        let epoch = epoch_borrow.wrapping_add(1);
        *epoch_borrow = epoch;
        drop(epoch_borrow);

        let reset_listeners = self.reset_listeners.read().unwrap();
        for listener_group in reset_listeners.iter() {
            let listeners = listener_group.read().unwrap();
            for listener in listeners.iter() {
                listener(epoch);
            }
        }
    }

    pub fn add_reset_listener_group(&self, listener: Arc<RwLock<Vec<ResetPipelineFunction>>>) {
        let mut reset_listeners = self.reset_listeners.write().unwrap();
        reset_listeners.push(listener);
    }

    pub fn current_epoch(&self) -> u16 {
        *self.epoch.read().unwrap()
    }

    pub fn current_epoch_ref(&self) -> Arc<RwLock<u16>> {
        self.epoch.clone()
    }
}
