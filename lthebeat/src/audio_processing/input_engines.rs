use crate::audio_processing::audio_pipeline::faucet::Faucet;
use crate::audio_processing::input_engines::symphonia_engine::SymphoniaEngine;
use crate::play_queue::media_item::MediaItem;
use gpui::Entity;
use std::time::Duration;
use url::Url;

pub mod symphonia_engine;

pub fn faucet_for_url(
    url: Url,
    associated_track: Option<Entity<MediaItem>>,
) -> Option<Box<dyn Controller>> {
    if let Ok(symphonia_engine) = SymphoniaEngine::new(url, associated_track) {
        return Some(Box::new(symphonia_engine));
    };

    None
}

pub trait Controller {
    fn faucet(&mut self) -> Faucet;
    fn seek(&mut self, position: Duration);
}
