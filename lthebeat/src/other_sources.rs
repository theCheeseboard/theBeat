use gpui::{AnyElement, ElementId, Global};

pub trait OtherSource {
    fn render(&self) -> AnyElement;
    fn name(&self) -> &str;
    fn id(&self) -> ElementId;
}

pub struct OtherSourcesManager {
    other_sources: Vec<Box<dyn OtherSource>>
}

impl OtherSourcesManager {
    pub fn new() -> Self {
        Self {
            other_sources: Vec::new()
        }
    }

    pub fn push_source(&mut self, source: Box<dyn OtherSource>) {
        self.other_sources.push(source)
    }

    pub fn sources(&self) -> &Vec<Box<dyn OtherSource>> {
        &self.other_sources
    }
}

impl Global for OtherSourcesManager {}