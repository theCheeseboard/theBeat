use crate::audio_processing::audio_pipeline::faucet::Faucet;
use crate::audio_processing::input_engines::symphonia_engine::SymphoniaEngine;
use crate::play_queue::media_item::MediaItem;
use gpui::Entity;
use url::Url;

pub mod symphonia_engine;

pub fn faucet_for_url(url: Url, associated_track: Option<Entity<MediaItem>>) -> Option<Faucet> {
    if let Ok(mut symphonia_engine) = SymphoniaEngine::new(url, associated_track) {
        return Some(symphonia_engine.faucet());
    };

    None
}
