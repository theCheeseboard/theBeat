use crate::audio_processing::audio_pipeline::faucet::Faucet;
use crate::audio_processing::input_engines::symphonia_engine::SymphoniaEngine;
use url::Url;

pub mod symphonia_engine;

pub fn faucet_for_url(url: Url) -> Option<Faucet> {
    if let Ok(mut symphonia_engine) = SymphoniaEngine::new(url) {
        return Some(symphonia_engine.faucet());
    };

    None
}
