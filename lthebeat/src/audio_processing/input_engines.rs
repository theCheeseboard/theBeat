use crate::audio_processing::audio_pipeline::faucet::Faucet;
use crate::play_queue::media_item::MediaItem;
use gpui::{App, BorrowAppContext, Entity, Global};
use std::time::Duration;
use url::Url;

pub mod symphonia_engine;

pub struct EngineManager {
    factories: Vec<Box<dyn EngineFactory>>
}

impl EngineManager {
    pub fn new() -> Self {
        Self {
            factories: Vec::new()
        }
    }
    
    pub fn register_factory(&mut self, factory: impl EngineFactory + 'static) {
        self.factories.push(Box::new(factory));
    }
    
    pub fn factories(&self) -> &Vec<Box<dyn EngineFactory>> {
        &self.factories
    }
}

impl Global for EngineManager {}

pub trait EngineFactory {
    fn faucet_for_url(&self, url: Url, associated_track: Option<Entity<MediaItem>>, cx: &mut App) -> Option<Box<dyn Controller>>;
}

pub fn faucet_for_url(
    url: Url,
    associated_track: Option<Entity<MediaItem>>,
    cx: &mut App
) -> Option<Box<dyn Controller>> {
    cx.update_global::<EngineManager, _>(|engine_manager, cx| {
        for factory in engine_manager.factories() {
            if let Some(engine) = factory.faucet_for_url(url.clone(), associated_track.clone(), cx) {
                return Some(engine);
            }
        }
    
        None
    })
}

pub trait Controller {
    fn faucet(&mut self) -> Faucet;
    fn seek(&mut self, position: Duration);
}
