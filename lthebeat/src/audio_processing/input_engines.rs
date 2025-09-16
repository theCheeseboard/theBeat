use crate::audio_processing::audio_pipeline::faucet::Faucet;
use crate::audio_processing::input_engines::symphonia_engine::SymphoniaEngine;
use crate::play_queue::media_item::MediaItem;
use gpui::{App, Entity};
use std::time::Duration;
use url::Url;

pub mod symphonia_engine;

#[cfg(target_os = "linux")]
pub mod cdio_paranoia_engine;

pub fn faucet_for_url(
    url: Url,
    associated_track: Option<Entity<MediaItem>>,
    cx: &mut App
) -> Option<Box<dyn Controller>> {
    if let Ok(symphonia_engine) = SymphoniaEngine::new(url.clone(), associated_track.clone()) {
        return Some(Box::new(symphonia_engine));
    };

    #[cfg(target_os = "linux")]
    if let Ok(cdio_paranoia_engine) = cdio_paranoia_engine::CdioParanoiaEngine::new(url.clone(), associated_track.clone(), cx) {
        return Some(Box::new(cdio_paranoia_engine));
    }

    None
}

pub trait Controller {
    fn faucet(&mut self) -> Faucet;
    fn seek(&mut self, position: Duration);
}
