use crate::platform::PlatformHandler;
use gpui::{App, AppContext, Entity};

struct LinuxPlatform {}

impl PlatformHandler for LinuxPlatform {}

pub fn create_platform(cx: &mut App) -> Entity<Box<dyn PlatformHandler>> {
    cx.new(|_| -> Box<dyn PlatformHandler> { Box::new(LinuxPlatform {}) })
}
