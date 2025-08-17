pub mod media_item;

use std::sync::{Arc, RwLock};
use std::time::Duration;
use async_ringbuf::traits::AsyncProducer;
use crate::cyclic_cursor_vec::CyclicCursorVec;
use crate::play_queue::media_item::MediaItem;
use gpui::{App, AppContext, AsyncApp, Entity, Global};
use url::Url;
use rand::{random, random_range};
use crate::audio_processing::audio_pipeline::duplicator::Duplicator;
use crate::audio_processing::audio_pipeline::faucet::{create_faucet, Faucet};
use crate::audio_processing::audio_pipeline::plug;
use crate::audio_processing::input_engines::faucet_for_url;

pub struct PlayQueue {
    shown_items: Vec<Entity<MediaItem>>,
    played_items: Arc<RwLock<CyclicCursorVec<Entity<MediaItem>>>>,
    shuffle: bool,
    faucet_queue: Arc<RwLock<Vec<RwLock<Faucet>>>>,
    duplicator: Duplicator
}

impl PlayQueue {
    pub fn new(cx: &mut App) -> Self {
        let (faucet, mut faucet_input) = create_faucet();
        let mut duplicator = Duplicator::new();

        plug(faucet, duplicator.sink());

        let faucet_queue = Arc::new(RwLock::new(Vec::new()));
        let played_items = Arc::new(RwLock::new(CyclicCursorVec::new()));

        let play_queue = Self {
            shown_items: Vec::new(),
            played_items: played_items.clone(),
            shuffle: false,
            faucet_queue: faucet_queue.clone(),
            duplicator
        };

        cx.spawn(async move |cx: &mut AsyncApp| {
            loop {
                let mut faucet_queue = faucet_queue.write().unwrap();

                while faucet_queue.len() < 2 {
                    // Push a new faucet onto the queue
                    let mut played_items = played_items.write().unwrap();
                    if played_items.is_empty() {
                        break;
                    }

                    let next_media_item = played_items.next();

                    if let Some(faucet) = next_media_item.update(cx, |next_media_item, cx| {
                        faucet_for_url(next_media_item.url.clone())
                    }).ok().flatten() {
                        faucet_queue.push(RwLock::new(faucet))
                    }
                }

                if faucet_queue.is_empty() {
                    // TODO: better way to do this
                    smol::Timer::after(Duration::from_millis(100)).await;
                    continue;
                }

                let mut next_sample = None;
                {
                    let mut faucet_arc = faucet_queue.first().unwrap().write().unwrap();
                    if faucet_arc.samples_waiting() > 0 {
                        next_sample = Some(smol::block_on(faucet_arc.next_sample()));
                    }
                }

                let Some(next_sample) = next_sample else {
                    // TODO: better way to do this
                    smol::Timer::after(Duration::from_millis(100)).await;
                    continue;
                };

                if next_sample.is_ok() {
                    faucet_input.push(next_sample).await.unwrap();
                } else {
                    faucet_queue.remove(0);
                }
            }
        }).detach();

        play_queue
    }

    pub fn add_item(&mut self, item: Entity<MediaItem>) {
        self.shown_items.push(item.clone());

        let mut played_items = self.played_items.write().unwrap();

        if self.shuffle {
            let new_index = random_range(0..played_items.len());
            played_items.insert(new_index, item);
        } else {
            played_items.push(item);
        }
    }

    pub fn open_faucet(&mut self) -> Faucet {
        self.duplicator.open_faucet()
    }
}

impl Global for PlayQueue {}