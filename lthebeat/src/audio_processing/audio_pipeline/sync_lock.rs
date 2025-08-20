use std::ptr::write;
use crate::audio_processing::audio_pipeline::{PipelineSample, PipelineSampleResult, Sample, SAMPLE_BUFFER_SIZE};
use std::sync::{Arc, RwLock};
use async_channel::{Receiver, RecvError, Sender};
use async_ringbuf::{AsyncHeapCons, AsyncHeapProd, AsyncHeapRb, AsyncRb};
use async_ringbuf::traits::{AsyncProducer, Split};
use log::warn;
use rand::random;
use smol::stream::StreamExt;
use crate::audio_processing::audio_pipeline::faucet::{create_faucet, Faucet};
use crate::audio_processing::audio_pipeline::sink::{create_sink, Sink};

pub struct SyncLock {
    pub(crate) packet_buffer: Receiver<PipelineSample>,
    pub(crate) write_buffer: Sender<PipelineSample>,
    pub(crate) is_under_management: RwLock<bool>,
    pub(crate) id: u64
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

        let (mut packet_buffer_prod, packet_buffer_cons) = async_channel::bounded(SAMPLE_BUFFER_SIZE);
        let (write_buffer_prod, write_buffer_cons) = async_channel::unbounded();

        let sync_lock = Arc::new(SyncLock {
            packet_buffer: packet_buffer_cons,
            write_buffer: write_buffer_prod.clone(),
            is_under_management: RwLock::new(false),
            id: random()
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
        }).detach();
        smol::spawn(async move {
            loop {
                match write_buffer_cons.recv().await {
                    Ok(next_packet) => {
                        rb_faucet_prod.push(Ok(next_packet)).await.unwrap();
                    }
                    Err(_) => {
                        warn!("could not receive packet");
                        return
                    }
                }
            }
        }).detach();

        SyncLockCreate {
            sink,
            faucet,
            sync_lock
        }
    }
}