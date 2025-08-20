use std::collections::HashMap;
use std::sync::{Arc, RwLock};
use std::time::Duration;
use async_channel::RecvError;
use async_ringbuf::traits::{AsyncProducer, Consumer};
use log::{error, warn};
use smol::io::AsyncReadExt;
use smol::stream::{pending, StreamExt};
use crate::audio_processing::audio_metadata::AudioMetadata;
use crate::audio_processing::audio_pipeline::PipelineSample;
use crate::audio_processing::audio_pipeline::sync_lock::SyncLock;

pub struct SyncLockSync {
    current_meta: Arc<RwLock<AudioMetadata>>,
    under_management: Arc<RwLock<Vec<Arc<SyncLock>>>>,
}

impl Default for SyncLockSync {
    fn default() -> Self {
        Self::new()
    }
}

impl SyncLockSync {
    pub fn new() -> SyncLockSync {
        let under_management = Arc::new(RwLock::new(Vec::new()));
        let current_meta = Arc::new(RwLock::default());
        let sync_lock_sync = SyncLockSync {
            current_meta: current_meta.clone(),
            under_management: under_management.clone(),
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
                        Ok(PipelineSample::Reset) => {
                            // Propagate the reset packet
                            sync_lock.write_buffer.send(PipelineSample::Reset).await.unwrap();
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
                    for sync_lock in under_management.iter() {
                        let packet = crate_packets.remove(&sync_lock.id).expect("All sync locks accounted for but found sync lock without corresponding packet");
                        // Update the current sample ID
                        current_sample_id = Some(packet.sample_id);

                        // Update the metadata
                        meta = packet.meta.clone();

                        // Send out the packet
                        sync_lock.write_buffer.send(PipelineSample::Sample(packet)).await.unwrap();
                    }

                    println!("Playing sample from {:?} id {:?}", meta, current_sample_id.unwrap());
                    *current_meta.write().unwrap() = meta;
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
        self.under_management.write().unwrap().push(sync_lock.clone());
        *is_under_management = true;
    }

    pub fn unmanage(&mut self, sync_lock: Arc<SyncLock>) {
        let mut is_under_management = sync_lock.is_under_management.write().unwrap();
        *is_under_management = false;

        let mut under_management = self.under_management.write().unwrap();
        let index = under_management.iter().position(|s| Arc::ptr_eq(s, &sync_lock)).unwrap();
        under_management.remove(index);
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