use crate::audio_processing::audio_metadata::AudioMetadata;
use gpui::{App, AppContext, Entity, Global};
use std::any::Any;
use std::time::Duration;

#[cfg(target_os = "linux")]
mod linux;

#[cfg(target_os = "macos")]
mod macos;

#[cfg(target_os = "windows")]
mod win;

pub trait PlatformHandler: Any {
    fn new_metadata_available(&mut self, meta: AudioMetadata, cx: &mut App) {}
    fn play_state_changed(&mut self, is_playing: bool, cx: &mut App) {}
    fn seek_performed(&mut self, current_time: Duration, cx: &mut App) {}

    fn as_any(&self) -> &dyn Any;
    fn as_any_mut(&mut self) -> &mut dyn Any;
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
    fn play_state_changed(&mut self, is_playing: bool, cx: &mut App) {
        cx.update_entity(&self.inner, |inner, cx| inner.play_state_changed(is_playing, cx))
    }
    fn seek_performed(&mut self, current_time: Duration, cx: &mut App) {
        cx.update_entity(&self.inner, |inner, cx| {
            inner.seek_performed(current_time, cx)
        })
    }

    fn as_any(&self) -> &dyn Any {
        self
    }

    fn as_any_mut(&mut self) -> &mut dyn Any {
        self
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
