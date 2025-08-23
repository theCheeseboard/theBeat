pub mod media_item;

use crate::audio_processing::audio_pipeline::duplicator::Duplicator;
use crate::audio_processing::audio_pipeline::faucet::{Faucet, create_faucet};
use crate::audio_processing::audio_pipeline::{PipelineSample, PipelineSampleResult, plug};
use crate::audio_processing::input_engines::faucet_for_url;
use crate::cyclic_cursor_vec::CyclicCursorVec;
use crate::play_queue::media_item::MediaItem;
use async_lock::RwLock;
use async_ringbuf::AsyncHeapProd;
use async_ringbuf::traits::AsyncProducer;
use gpui::{App, AsyncApp, Entity, Global};
use rand::random_range;
use std::sync::Arc;
use std::time::Duration;

struct FaucetQueueItem {
    associated_item: Entity<MediaItem>,
    faucet: Faucet,
}

pub struct PlayQueue {
    pub shown_items: Vec<Entity<MediaItem>>,
    played_items: Arc<RwLock<CyclicCursorVec<Entity<MediaItem>>>>,
    shuffle: bool,
    faucet_queue: Arc<RwLock<Vec<FaucetQueueItem>>>,
    faucet_input: Arc<RwLock<AsyncHeapProd<PipelineSampleResult>>>,
    duplicator: Duplicator,
}

impl PlayQueue {
    pub fn new(cx: &mut App) -> Self {
        let (faucet, faucet_input) = create_faucet();
        let mut duplicator = Duplicator::new();

        let faucet_input = Arc::new(RwLock::new(faucet_input));

        plug(faucet, duplicator.sink());

        let faucet_queue = Arc::new(RwLock::new(Vec::new()));
        let played_items = Arc::new(RwLock::new(CyclicCursorVec::new()));

        let play_queue = Self {
            shown_items: Vec::new(),
            played_items: played_items.clone(),
            shuffle: false,
            faucet_queue: faucet_queue.clone(),
            faucet_input: faucet_input.clone(),
            duplicator,
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

                    let next_media_item = played_items.next();

                    if let Some(faucet) = next_media_item
                        .update(cx, |next_media_item, _| {
                            faucet_for_url(next_media_item.url.clone())
                        })
                        .ok()
                        .flatten()
                    {
                        faucet_queue_borrow.push(FaucetQueueItem {
                            faucet,
                            associated_item: next_media_item.clone(),
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
                    faucet_input.write().await.push(next_sample).await.unwrap();
                } else {
                    let mut faucet_queue_borrow = faucet_queue.write().await;
                    faucet_queue_borrow.remove(0);
                }
            }
        })
            .detach();

        play_queue
    }

    pub fn add_item(&mut self, item: Entity<MediaItem>) {
        self.shown_items.push(item.clone());

        let mut played_items = self.played_items.write_blocking();

        if self.shuffle {
            let new_index = random_range(0..played_items.len());
            played_items.insert(new_index, item);
        } else {
            played_items.push(item);
        }

        let mut faucet_queue_borrow = self.faucet_queue.write_blocking();

        let current_item = played_items.current();
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

                let faucet_input = self.faucet_input.clone();
                smol::spawn(async move {
                    faucet_input
                        .write()
                        .await
                        .push(Ok(PipelineSample::Reset))
                        .await
                        .unwrap();
                })
                    .detach()
            } else {
                // Enqueue new faucets
                while faucet_queue_borrow.len() >= 2 {
                    faucet_queue_borrow.remove(1);
                }
            }
        }
    }

    pub fn open_faucet(&mut self) -> Faucet {
        self.duplicator.open_faucet()
    }

    pub fn skip_next(&mut self) {
        let mut faucet_queue_borrow = self.faucet_queue.write_blocking();
        if !faucet_queue_borrow.is_empty() {
            faucet_queue_borrow.remove(0);

            let faucet_input = self.faucet_input.clone();
            smol::spawn(async move {
                faucet_input
                    .write()
                    .await
                    .push(Ok(PipelineSample::Reset))
                    .await
                    .unwrap();
            })
                .detach()
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

        let faucet_input = self.faucet_input.clone();
        smol::spawn(async move {
            faucet_input
                .write()
                .await
                .push(Ok(PipelineSample::Reset))
                .await
                .unwrap();
        })
            .detach()
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

        let faucet_input = self.faucet_input.clone();
        smol::spawn(async move {
            faucet_input
                .write()
                .await
                .push(Ok(PipelineSample::Reset))
                .await
                .unwrap();
        })
            .detach()
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
}

impl Global for PlayQueue {}

#[derive(Clone)]
pub enum DisplayQueueItem {
    SingleItemGroup(Entity<MediaItem>),
    GroupItem(Entity<MediaItem>),
    GroupHeader(Entity<MediaItem>),
}
