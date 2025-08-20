use crate::audio_processing::audio_pipeline::faucet::{Faucet, create_faucet};
use crate::audio_processing::audio_pipeline::sink::{Sink, create_sink};
use crate::audio_processing::audio_pipeline::{PipelineSample, SAMPLE_BUFFER_SIZE};
use async_channel::{Receiver, Sender};
use async_ringbuf::traits::AsyncProducer;
use log::warn;
use rand::random;
use smol::stream::StreamExt;
use std::sync::{Arc, RwLock};

pub struct SyncLock {
    pub(crate) packet_buffer: Receiver<PipelineSample>,
    pub(crate) write_buffer: Sender<PipelineSample>,
    pub(crate) is_under_management: RwLock<bool>,
    pub(crate) id: u64,
}

pub struct SyncLockCreate {
    pub sink: Sink,
    pub faucet: Faucet,
    pub sync_lock: Arc<SyncLock>,
}

impl SyncLock {
    pub fn create_sync_lock() -> SyncLockCreate {
        let (faucet, mut rb_faucet_prod) = create_faucet();
        let (sink, mut rb_sink_cons) = create_sink();

        let (packet_buffer_prod, packet_buffer_cons) = async_channel::bounded(SAMPLE_BUFFER_SIZE);
        let (write_buffer_prod, write_buffer_cons) = async_channel::bounded(SAMPLE_BUFFER_SIZE);

        let sync_lock = Arc::new(SyncLock {
            packet_buffer: packet_buffer_cons,
            write_buffer: write_buffer_prod.clone(),
            is_under_management: RwLock::new(false),
            id: random(),
        });

        smol::spawn(async move {
            loop {
                match rb_sink_cons.next().await {
                    Some(Ok(pipeline_sample)) => {
                        packet_buffer_prod.send(pipeline_sample).await.unwrap();
                    }
                    _ => {
                        return;
                    }
                }
            }
        })
            .detach();
        smol::spawn(async move {
            loop {
                match write_buffer_cons.recv().await {
                    Ok(next_packet) => {
                        rb_faucet_prod.push(Ok(next_packet)).await.unwrap();
                    }
                    Err(_) => {
                        warn!("could not receive packet");
                        return;
                    }
                }
            }
        })
            .detach();

        SyncLockCreate {
            sink,
            faucet,
            sync_lock,
        }
    }
}
