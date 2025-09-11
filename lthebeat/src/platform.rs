use crate::audio_processing::audio_metadata::AudioMetadata;
use gpui::{App, AppContext, Entity, Global};

mod linux;
mod macos;
mod win;

pub trait PlatformHandler {
    fn new_metadata_available(&mut self, meta: AudioMetadata, cx: &mut App) {}
    fn play_state_changed(&mut self, cx: &mut App) {}
}

pub struct Platform {
    inner: Entity<Box<dyn PlatformHandler>>,
}

impl Platform {
    fn new(cx: &mut App) -> Platform {
        Platform {
            inner: create_platform(cx),
        }
    }
}

impl PlatformHandler for Platform {
    fn new_metadata_available(&mut self, meta: AudioMetadata, cx: &mut App) {
        cx.update_entity(&self.inner, |inner, cx| {
            inner.new_metadata_available(meta, cx)
        })
    }
    fn play_state_changed(&mut self, cx: &mut App) {
        cx.update_entity(&self.inner, |inner, cx| inner.play_state_changed(cx))
    }
}

impl Global for Platform {}

#[allow(unreachable_code)]
fn create_platform(cx: &mut App) -> Entity<Box<dyn PlatformHandler>> {
    #[cfg(target_os = "macos")]
    return macos::create_platform(cx);

    #[cfg(target_os = "linux")]
    return linux::create_platform(cx);

    #[cfg(target_os = "windows")]
    return win::create_platform(cx);

    panic!("Unsupported platform");
}

pub fn setup_platform(cx: &mut App) {
    let platform = Platform::new(cx);
    cx.set_global::<Platform>(platform);
}
