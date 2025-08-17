use gpui::{App, AppContext, Entity};
use url::Url;

pub struct MediaItem {
    pub url: Url
}

impl MediaItem {
    pub fn new(url: Url, cx: &mut App) -> Entity<Self> {
        cx.new(|_| {
            MediaItem {
                url
            }
        })
    }
}