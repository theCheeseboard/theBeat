use std::sync::{Arc, RwLock};

#[derive(Clone)]
pub struct Resetter {
    reset_listeners: Arc<RwLock<Vec<Arc<RwLock<Vec<Box<dyn Fn() + Send + Sync>>>>>>>,
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
        }
    }

    pub fn trigger_reset(&self) {
        let reset_listeners = self.reset_listeners.read().unwrap();
        for listener_group in reset_listeners.iter() {
            let listeners = listener_group.read().unwrap();
            for listener in listeners.iter() {
                listener();
            }
        }
    }

    pub fn add_reset_listener_group(
        &self,
        listener: Arc<RwLock<Vec<Box<dyn Fn() + Send + Sync>>>>,
    ) {
        let mut reset_listeners = self.reset_listeners.write().unwrap();
        reset_listeners.push(listener);
    }
}
