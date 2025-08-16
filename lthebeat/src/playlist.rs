mod media_item;

use crate::cyclic_cursor_vec::CyclicCursorVec;
use crate::playlist::media_item::MediaItem;
use gpui::{App, AppContext, Entity, Global};
use url::Url;
use rand::{random, random_range};
use crate::audio_processing::audio_pipeline::faucet::Faucet;

pub struct Playlist {
    shown_items: Vec<Entity<MediaItem>>,
    played_items: CyclicCursorVec<Entity<MediaItem>>,
    shuffle: bool,
    faucets: Vec<Faucet>
}

impl Default for Playlist {
    fn default() -> Self {
        Self::new()
    }
}

impl Playlist {
    pub fn new() -> Self {
        Self {
            shown_items: Vec::new(),
            played_items: CyclicCursorVec::new(),
            shuffle: false,
            faucets: Vec::new()
        }
    }

    pub fn add_item(&mut self, url: Url, cx: &mut App) {
        let item = cx.new(|_| {
            MediaItem {
                url
            }
        });
        self.shown_items.push(item.clone());

        if self.shuffle {
            let new_index = random_range(0..self.played_items.len());
            self.played_items.insert(new_index, item);
        } else {
            self.played_items.push(item);
        }
    }
}

impl Global for Playlist {}