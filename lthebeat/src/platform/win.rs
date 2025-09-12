use crate::platform::PlatformHandler;
use gpui::{App, AppContext, Entity};
use std::any::Any;

struct WinPlatform {}

impl PlatformHandler for WinPlatform {
    fn as_any(&self) -> &dyn Any { self }
    fn as_any_mut(&mut self) -> &mut dyn Any { self }
}

pub fn create_platform(cx: &mut App) -> Entity<Box<dyn PlatformHandler>> {
    cx.new(|_| -> Box<dyn PlatformHandler> { Box::new(WinPlatform {}) })
}
