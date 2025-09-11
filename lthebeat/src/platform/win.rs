use crate::platform::PlatformHandler;
use gpui::{App, AppContext, Entity};

struct WinPlatform {}

impl PlatformHandler for WinPlatform {}

pub fn create_platform(cx: &mut App) -> Entity<Box<dyn PlatformHandler>> {
    cx.new(|_| -> Box<dyn PlatformHandler> { Box::new(WinPlatform {}) })
}
