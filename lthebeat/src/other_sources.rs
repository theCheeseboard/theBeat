use gpui::{AnyElement, ElementId, Global};
use uuid::Uuid;

pub trait OtherSource {
    fn render(&self) -> AnyElement;
    fn name(&self) -> &str;
    fn id(&self) -> ElementId;
}

pub struct OtherSourcesManager {
    other_sources: Vec<SourceDescriptor>,
}

struct SourceDescriptor {
    uuid: Uuid,
    source: Box<dyn OtherSource>,
}

impl OtherSourcesManager {
    pub fn new() -> Self {
        Self {
            other_sources: Vec::new(),
        }
    }

    pub fn push_source(&mut self, source: Box<dyn OtherSource>) -> Uuid {
        let uuid = Uuid::new_v4();
        self.other_sources.push(SourceDescriptor { uuid, source });
        uuid
    }
    
    pub fn remove_source(&mut self, id: Uuid) {
        self.other_sources.retain(|source| source.uuid != id);
    }

    pub fn sources(&self) -> impl Iterator<Item = &dyn OtherSource> {
        self.other_sources
            .iter()
            .map(|source| source.source.as_ref())
    }
}

impl Global for OtherSourcesManager {}
