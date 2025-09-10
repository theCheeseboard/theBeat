pub mod media_item;

use crate::audio_processing::audio_controller::AudioController;
use crate::audio_processing::audio_metadata::AudioMetadata;
use crate::audio_processing::audio_pipeline::duplicator::Duplicator;
use crate::audio_processing::audio_pipeline::faucet::{Faucet, create_faucet};
use crate::audio_processing::audio_pipeline::{
    PipelineSample, PipelineSampleExt, PipelineSampleResult, ResetPipelineFunction, plug,
};
use crate::audio_processing::input_engines::{Controller, faucet_for_url};
use crate::cyclic_cursor_vec::CyclicCursorVec;
use crate::play_queue::media_item::MediaItem;
use async_lock::RwLock;
use async_ringbuf::AsyncHeapProd;
use async_ringbuf::traits::{AsyncProducer, Consumer};
use gpui::{App, AppContext, AsyncApp, Entity, Global};
use rand::seq::SliceRandom;
use rand::{random_range, rng, thread_rng};
use smol::io::AsyncSeekExt;
use std::sync::Arc;
use std::time::Duration;

struct FaucetQueueItem {
    associated_item: Entity<MediaItem>,
    controller: Box<dyn Controller>,
    faucet: Faucet,
}

pub struct PlayQueue {
    pub shown_items: Vec<Entity<MediaItem>>,
    played_items: Arc<RwLock<CyclicCursorVec<Entity<MediaItem>>>>,
    faucet_queue: Arc<RwLock<Vec<FaucetQueueItem>>>,
    faucet_input: Arc<RwLock<AsyncHeapProd<PipelineSampleResult>>>,
    reset_faucet: Box<dyn Fn() + Send + Sync>,
    duplicator: Duplicator,

    pub shuffle: bool,
}

impl PlayQueue {
    pub fn new(cx: &mut App) -> Self {
        let (faucet, faucet_input, reset_faucet) = create_faucet();
        let mut duplicator = Duplicator::new();

        let faucet_input = Arc::new(RwLock::new(faucet_input));
        let epoch_ref = faucet.current_epoch();

        plug(faucet, duplicator.sink());

        let faucet_queue = Arc::new(RwLock::new(Vec::new()));
        let played_items = Arc::new(RwLock::new(CyclicCursorVec::new()));

        let play_queue = Self {
            shown_items: Vec::new(),
            played_items: played_items.clone(),
            faucet_queue: faucet_queue.clone(),
            faucet_input: faucet_input.clone(),
            reset_faucet,
            duplicator,
            shuffle: false,
        };

        cx.spawn(async move |cx: &mut AsyncApp| {
            loop {
                let mut faucet_queue_borrow = faucet_queue.write().await;

                while faucet_queue_borrow.len() < 2 {
                    // Push a new faucet onto the queue
                    let mut played_items = played_items.write().await;
                    if played_items.is_empty() {
                        break;
                    }

                    let next_media_item_entity = played_items.next();

                    if let Some(mut controller) = next_media_item_entity
                        .update(cx, |next_media_item, _| {
                            faucet_for_url(
                                next_media_item.url.clone(),
                                Some(next_media_item_entity.clone()),
                            )
                        })
                        .ok()
                        .flatten()
                    {
                        faucet_queue_borrow.push(FaucetQueueItem {
                            faucet: controller.faucet(),
                            controller,
                            associated_item: next_media_item_entity.clone(),
                        })
                    }
                }

                if faucet_queue_borrow.is_empty() {
                    drop(faucet_queue_borrow);
                    // TODO: better way to do this
                    smol::Timer::after(Duration::from_millis(100)).await;
                    continue;
                }

                let mut next_sample = None;
                {
                    let faucet_arc = &mut faucet_queue_borrow.first_mut().unwrap().faucet;
                    if faucet_arc.samples_waiting() > 0 {
                        next_sample = Some(smol::block_on(faucet_arc.next_sample()));
                    }
                }
                drop(faucet_queue_borrow);

                let Some(next_sample) = next_sample else {
                    // TODO: better way to do this
                    smol::Timer::after(Duration::from_millis(100)).await;
                    continue;
                };

                if next_sample.is_ok() {
                    faucet_input
                        .write()
                        .await
                        .push(next_sample.with_epoch_ref(&epoch_ref))
                        .await
                        .unwrap();
                } else {
                    let mut faucet_queue_borrow = faucet_queue.write().await;
                    faucet_queue_borrow.remove(0);
                }
            }
        })
        .detach();

        play_queue
    }

    pub fn add_item(&mut self, item: Entity<MediaItem>, cx: &mut App) {
        self.shown_items.push(item.clone());

        let mut played_items = self.played_items.write_blocking();

        if self.shuffle {
            let new_index = random_range(0..played_items.len());
            played_items.insert(new_index, item);
        } else {
            played_items.push(item);
        }
        drop(played_items);

        self.evict_faucets(cx);
    }

    pub fn open_faucet(&mut self) -> Faucet {
        self.duplicator.open_faucet()
    }

    pub fn skip_next(&mut self) {
        let mut faucet_queue_borrow = self.faucet_queue.write_blocking();
        if !faucet_queue_borrow.is_empty() {
            faucet_queue_borrow.remove(0);

            (self.reset_faucet)();
        }
    }

    pub fn skip_previous(&mut self) {
        let mut played_items = self.played_items.write_blocking();
        if played_items.is_empty() {
            return;
        }

        let mut faucet_queue_borrow = self.faucet_queue.write_blocking();
        for _ in 0..faucet_queue_borrow.len() {
            played_items.prev();
        }
        played_items.prev();
        faucet_queue_borrow.clear();

        (self.reset_faucet)();
    }

    pub fn skip_to_item(&mut self, item: Entity<MediaItem>) {
        let mut played_items = self.played_items.write_blocking();
        if played_items.is_empty() {
            return;
        }

        let new_position = played_items
            .vec
            .iter()
            .position(|i| i.entity_id() == item.entity_id())
            .unwrap();
        played_items.set_current(new_position);
        // Skip back again because the play thread will call next() on the current item
        played_items.prev();

        let mut faucet_queue_borrow = self.faucet_queue.write_blocking();
        faucet_queue_borrow.clear();

        (self.reset_faucet)();
    }

    pub fn clear(&mut self) {
        self.shown_items.clear();

        let mut played_items = self.played_items.write_blocking();
        played_items.clear();

        let mut faucet_queue_borrow = self.faucet_queue.write_blocking();
        faucet_queue_borrow.clear();

        (self.reset_faucet)();
    }

    pub fn remove_item(&mut self, item: Entity<MediaItem>) {
        self.shown_items
            .retain(|i| i.entity_id() != item.entity_id());

        let mut played_items = self.played_items.write_blocking();
        played_items.retain(|i| i.entity_id() != item.entity_id());

        let mut faucet_queue_borrow = self.faucet_queue.write_blocking();
        let flush_required = faucet_queue_borrow
            .first()
            .map(|i| i.associated_item.entity_id() == item.entity_id())
            .unwrap_or(false);
        faucet_queue_borrow.retain(|i| i.associated_item.entity_id() != item.entity_id());
        if flush_required {
            (self.reset_faucet)();
        }
    }

    pub fn seek_to_position(&mut self, position: Duration) {
        let mut faucet_queue_borrow = self.faucet_queue.write_blocking();
        if let Some(queue_item) = faucet_queue_borrow.first_mut() {
            (self.reset_faucet)();
            queue_item.controller.seek(position);
        }
    }

    pub fn display_queue(&self, cx: &App) -> Vec<DisplayQueueItem> {
        let mut queue = Vec::new();

        let mut current_group_album = None;
        let mut peekable = self.shown_items.iter().peekable();
        while let Some(item_entity) = peekable.next() {
            let item = item_entity.read(cx);
            if item.meta.album == current_group_album && current_group_album.is_some() {
                queue.push(DisplayQueueItem::GroupItem(item_entity.clone()));
            } else {
                let next_item = peekable.peek().map(|item| item.read(cx));
                if let Some(next_item) = next_item
                    && next_item.meta.album == item.meta.album
                {
                    queue.push(DisplayQueueItem::GroupHeader(item_entity.clone()));
                    queue.push(DisplayQueueItem::GroupItem(item_entity.clone()));
                    current_group_album = next_item.meta.album.clone()
                } else {
                    queue.push(DisplayQueueItem::SingleItemGroup(item_entity.clone()));
                    current_group_album = None;
                }
            }
        }

        queue
    }

    pub fn repeat_one(&self) -> bool {
        self.played_items.read_blocking().repeat_one
    }

    pub fn set_repeat_one(&mut self, repeat_one: bool, cx: &mut App) {
        let mut played_items = self.played_items.write_blocking();
        played_items.repeat_one(repeat_one);
        drop(played_items);

        self.evict_faucets(cx);
    }

    pub fn shuffle(&mut self, shuffle: bool, cx: &mut App) {
        self.shuffle = shuffle;

        let playing_track = playing_track(cx);
        if shuffle {
            let mut played_items = self.played_items.write_blocking();
            played_items.vec.shuffle(&mut rng());

            let playing_track_index = playing_track
                .and_then(|playing_track| {
                    played_items
                        .vec
                        .iter()
                        .position(|i| i.entity_id() == playing_track.entity_id())
                })
                .unwrap_or_default();
            played_items.set_current(playing_track_index)
        } else {
            let playing_track_index = playing_track
                .and_then(|playing_track| {
                    self.shown_items
                        .iter()
                        .position(|i| i.entity_id() == playing_track.entity_id())
                })
                .unwrap_or_default();
            let mut played_items = self.played_items.write_blocking();
            played_items.set_vec(self.shown_items.clone(), playing_track_index)
        }

        self.evict_faucets(cx);
    }

    fn evict_faucets(&mut self, cx: &mut App) {
        let mut faucet_queue_borrow = self.faucet_queue.write_blocking();

        match playing_track(cx) {
            Some(current_item) => {
                if !faucet_queue_borrow.is_empty() {
                    if faucet_queue_borrow
                        .first()
                        .unwrap()
                        .associated_item
                        .entity_id()
                        != current_item.entity_id()
                    {
                        // We've already started streaming the next item, so reset everything and jump straight to the next item
                        faucet_queue_borrow.clear();

                        (self.reset_faucet)();
                    } else {
                        // Enqueue new faucets
                        while faucet_queue_borrow.len() >= 2 {
                            faucet_queue_borrow.remove(1);
                        }
                    }
                }

                let mut played_items = self.played_items.write_blocking();
                if let Some(new_position) = played_items
                    .vec
                    .iter()
                    .position(|i| i.entity_id() == current_item.entity_id())
                {
                    played_items.set_current(new_position);
                }
            }
            None => {
                // Enqueue new faucets
                while faucet_queue_borrow.len() >= 2 {
                    faucet_queue_borrow.remove(1);
                }
            }
        }
    }
}

impl Global for PlayQueue {}

#[derive(Clone)]
pub enum DisplayQueueItem {
    SingleItemGroup(Entity<MediaItem>),
    GroupItem(Entity<MediaItem>),
    GroupHeader(Entity<MediaItem>),
}

fn playing_track(cx: &mut App) -> Option<Entity<MediaItem>> {
    let audio_controller = cx.global::<AudioController>();
    audio_controller.current_metadata().associated_item
}
