use std::collections::HashMap;
use gpui::Global;
use url::Url;
use crate::audio_processing::audio_metadata::AudioMetadata;

#[derive(Default)]
pub struct MetadataRegistry {
    metadata: HashMap<Url, AudioMetadata>
}

impl MetadataRegistry {
    pub fn metadata(&self, url: &Url) -> Option<&AudioMetadata> {
        self.metadata.get(url)
    }
    
    pub fn insert_metadata(&mut self, url: Url, metadata: AudioMetadata) {
        self.metadata.insert(url, metadata);
    }
    
    pub fn remove_metadata(&mut self, url: &Url) {
        self.metadata.remove(url);
    }
}

impl Global for MetadataRegistry {}