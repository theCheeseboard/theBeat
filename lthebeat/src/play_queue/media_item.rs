use crate::audio_processing::audio_metadata::AudioMetadata;
use crate::audio_processing::input_engines::symphonia_engine::SymphoniaEngine;
use crate::metadata_registry::MetadataRegistry;
use gpui::{App, AppContext, AsyncApp, Context, Entity};
use url::Url;

pub struct MediaItem {
    pub url: Url,
    symphonia_meta: AudioMetadata,
    metadata_registry_metadata: Option<AudioMetadata>,
}

impl MediaItem {
    pub fn new(url: Url, cx: &mut Context<Self>) -> Self {
        cx.spawn({
            let url = url.clone();
            async move |weak_this, cx: &mut AsyncApp| {
                if let Ok(meta) = SymphoniaEngine::audio_metadata(url.clone()).await {
                    let _ = weak_this.update(cx, |this, cx| {
                        this.symphonia_meta = meta;
                        cx.notify()
                    });
                }
            }
        })
        .detach();

        cx.observe_global::<MetadataRegistry>({
            let url = url.clone();
            move |this, cx| {
                let metadata_registry = cx.global::<MetadataRegistry>();
                this.metadata_registry_metadata = metadata_registry.metadata(&url).cloned();
                cx.notify()
            }
        })
        .detach();

        let metadata_registry = cx.global::<MetadataRegistry>();

        MediaItem {
            url: url.clone(),
            symphonia_meta: AudioMetadata {
                url: Some(url.clone()),
                ..AudioMetadata::default()
            },
            metadata_registry_metadata: metadata_registry.metadata(&url).cloned(),
        }
    }

    pub fn meta(&self) -> &AudioMetadata {
        self.metadata_registry_metadata
            .as_ref()
            .unwrap_or(&self.symphonia_meta)
    }
}
