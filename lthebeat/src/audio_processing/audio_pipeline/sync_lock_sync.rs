use crate::audio_processing::audio_metadata::AudioMetadata;
use crate::audio_processing::audio_pipeline::PipelineSample;
use crate::audio_processing::audio_pipeline::sync_lock::SyncLock;
use crate::play_queue::media_item::MediaItem;
use async_channel::Receiver;
use gpui::Entity;
use log::warn;
use smol::io::AsyncWriteExt;
use std::collections::HashMap;
use std::sync::{Arc, RwLock, mpsc};
use std::time::Duration;

pub struct SyncLockSync {
    pub current_meta: Arc<RwLock<AudioMetadata>>,
    pub current_time: Arc<RwLock<Option<Duration>>>,
    pub current_track: Arc<RwLock<Option<Entity<MediaItem>>>>,
    under_management: Arc<RwLock<Vec<Arc<SyncLock>>>>,
    event_channel: Receiver<()>,
}

impl Default for SyncLockSync {
    fn default() -> Self {
        Self::new()
    }
}

impl SyncLockSync {
    pub fn new() -> SyncLockSync {
        let (event_channel_sender, event_channel_receiver) = async_channel::bounded(1);

        let under_management = Arc::new(RwLock::new(Vec::new()));
        let current_meta = Arc::new(RwLock::default());
        let current_track = Arc::new(RwLock::default());
        let current_time = Arc::new(RwLock::default());
        let sync_lock_sync = SyncLockSync {
            current_meta: current_meta.clone(),
            current_time: current_time.clone(),
            current_track: current_track.clone(),
            under_management: under_management.clone(),
            event_channel: event_channel_receiver,
        };

        smol::spawn(async move {
            let mut current_sample_id = None;
            let mut crate_packets = HashMap::new();

            loop {
                let under_management = under_management.read().unwrap().clone();

                if under_management.is_empty() {
                    // TODO: better way to do this
                    smol::Timer::after(Duration::from_millis(100)).await;
                    continue;
                }

                // Wait for a new packet everywhere
                for sync_lock in under_management.iter() {
                    if crate_packets.contains_key(&sync_lock.id) {
                        continue;
                    }

                    // TODO: What happens if we start managing a new synclock and the it is out of sync?
                    match sync_lock.packet_buffer.recv().await {
                        Ok(PipelineSample::Sample(next_packet)) => {
                            crate_packets.insert(sync_lock.id, next_packet);
                        }
                        Err(_) => {
                            warn!("Unable to receive packet from SyncLock")
                        }
                    }
                }

                let all_sync_locks_accounted_for = crate_packets.len() == under_management.len();
                let mut packet_sent = false;
                if let Some(current_sample_id) = current_sample_id {
                    for sync_lock in under_management.iter() {
                        if let Some(packet) = crate_packets.get(&sync_lock.id) && current_sample_id == packet.sample_id {
                            // Send out the packet
                            sync_lock.write_buffer.send(PipelineSample::Sample(crate_packets.remove(&sync_lock.id).unwrap())).await.unwrap();
                            packet_sent = true;
                        }
                    }
                }

                if !packet_sent && all_sync_locks_accounted_for {
                    let mut meta = Default::default();
                    let mut elapsed_since_start = Default::default();
                    let mut associated_track = Default::default();
                    for sync_lock in under_management.iter() {
                        let packet = crate_packets.remove(&sync_lock.id).expect("All sync locks accounted for but found sync lock without corresponding packet");
                        // Update the current sample ID
                        current_sample_id = Some(packet.sample_id);

                        // Update the metadata
                        meta = packet.meta.clone();
                        elapsed_since_start = packet.elapsed_since_start;
                        associated_track = packet.associated_track.clone();

                        // Send out the packet
                        sync_lock.write_buffer.send(PipelineSample::Sample(packet)).await.unwrap();
                    }
                    *current_meta.write().unwrap() = meta;
                    *current_time.write().unwrap() = elapsed_since_start;
                    *current_track.write().unwrap() = associated_track;

                    let _ = event_channel_sender.try_send(());
                }
            }
        }).detach();

        sync_lock_sync
    }

    pub fn manage(&mut self, sync_lock: Arc<SyncLock>) {
        let mut is_under_management = sync_lock.is_under_management.write().unwrap();
        if *is_under_management {
            panic!("Tried to manage SyncLock twice");
        }
        self.under_management
            .write()
            .unwrap()
            .push(sync_lock.clone());
        *is_under_management = true;
    }

    pub fn unmanage(&mut self, sync_lock: Arc<SyncLock>) {
        let mut is_under_management = sync_lock.is_under_management.write().unwrap();
        *is_under_management = false;

        let mut under_management = self.under_management.write().unwrap();
        let index = under_management
            .iter()
            .position(|s| Arc::ptr_eq(s, &sync_lock))
            .unwrap();
        under_management.remove(index);
    }

    pub fn event_channel(&mut self) -> Receiver<()> {
        self.event_channel.clone()
    }
}

impl Drop for SyncLockSync {
    fn drop(&mut self) {
        let under_management = self.under_management.read().unwrap().clone();
        for sync_lock_under_management in under_management {
            self.unmanage(sync_lock_under_management.clone());
        }
    }
}
