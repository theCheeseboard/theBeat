use crate::audio_processing::audio_pipeline::faucet::{Faucet, create_faucet};
use crate::audio_processing::audio_pipeline::sink::{
    ResetListenerGroup, ResetListenerGroupTrait, Sink, create_sink,
};
use crate::audio_processing::audio_pipeline::{
    PipelineSample, PipelineSampleExt, SAMPLE_BUFFER_SIZE,
};
use async_channel::{Receiver, Sender};
use async_ringbuf::traits::{AsyncProducer, Consumer};
use log::warn;
use rand::random;
use smol::stream::StreamExt;
use std::sync::{Arc, RwLock};

pub struct SyncLock {
    pub(crate) packet_buffer: Receiver<PipelineSample>,
    pub(crate) write_buffer: Sender<PipelineSample>,
    pub(crate) reset_listeners: ResetListenerGroup,
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
        let (faucet, mut rb_faucet_prod, reset_faucet) = create_faucet();
        let (sink, mut rb_sink_cons) = create_sink();

        let reset_listeners = sink.reset_listeners();
        reset_listeners.add_reset_listener(Box::new(move |_| reset_faucet()));

        let reset_listeners_2 = reset_listeners.clone();
        let reset_listeners_3 = reset_listeners.clone();

        let (packet_buffer_prod, packet_buffer_cons) = async_channel::bounded(SAMPLE_BUFFER_SIZE);
        let (write_buffer_prod, write_buffer_cons) = async_channel::bounded(SAMPLE_BUFFER_SIZE);

        let sync_lock = Arc::new(SyncLock {
            packet_buffer: packet_buffer_cons,
            write_buffer: write_buffer_prod.clone(),
            reset_listeners,
            is_under_management: RwLock::new(false),
            id: random(),
        });

        smol::spawn(async move {
            let mut ct = reset_listeners_2.create_cancellation_token();
            loop {
                if ct.is_canceled() {
                    rb_sink_cons.clear();
                    ct = reset_listeners_2.create_cancellation_token();
                }

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

        let epoch_ref = faucet.current_epoch();
        smol::spawn(async move {
            let mut ct = reset_listeners_3.create_cancellation_token();
            loop {
                if ct.is_canceled() {
                    while !write_buffer_cons.is_empty() {
                        write_buffer_cons.recv().await.unwrap();
                    }
                    ct = reset_listeners_3.create_cancellation_token();
                }
                match write_buffer_cons.recv().await {
                    Ok(next_packet) => {
                        rb_faucet_prod
                            .push(Ok(next_packet.with_epoch_ref(&epoch_ref)))
                            .await
                            .unwrap();
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
