use crate::audio_processing::audio_metadata::AudioMetadata;
use crate::audio_processing::input_engines::symphonia_engine::SymphoniaEngine;
use gpui::{App, AppContext, AsyncApp, Entity};
use url::Url;

pub struct MediaItem {
    pub url: Url,
    pub meta: AudioMetadata,
}

impl MediaItem {
    pub fn new(url: Url, cx: &mut App) -> Entity<Self> {
        let entity = cx.new(|_| MediaItem {
            url: url.clone(),
            meta: AudioMetadata {
                url: Some(url.clone()),
                ..AudioMetadata::default()
            },
        });

        let entity_clone = entity.clone();
        cx.spawn(async move |cx: &mut AsyncApp| {
            if let Ok(meta) = SymphoniaEngine::audio_metadata(url.clone()).await {
                cx.update_entity(&entity_clone, |media_item, cx| {
                    media_item.meta = meta;
                    cx.notify()
                })
                    .unwrap();
            }
        })
            .detach();

        entity
    }
}
