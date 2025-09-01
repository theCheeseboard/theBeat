use crate::audio_processing::audio_pipeline::faucet::{Faucet, FaucetError, create_faucet};
use crate::audio_processing::audio_pipeline::sink::{
    ResetListenerGroup, ResetListenerGroupTrait, Sink, create_sink,
};
use crate::audio_processing::audio_pipeline::{PipelineSampleResult, SAMPLE_BUFFER_SIZE};
use async_channel::Sender;
use async_ringbuf::AsyncHeapRb;
use async_ringbuf::traits::{AsyncProducer, Consumer, Split};
use log::warn;
use rand::random;
use smol::stream::StreamExt;
use std::collections::HashMap;
use std::sync::{Arc, RwLock};
use std::time::Duration;

pub struct Duplicator {
    sink: Option<Sink>,
    faucets: Arc<RwLock<HashMap<u64, Sender<PipelineSampleResult>>>>,
    reset_listeners: ResetListenerGroup,
}

impl Default for Duplicator {
    fn default() -> Self {
        Self::new()
    }
}

impl Duplicator {
    pub fn new() -> Self {
        let (sink, mut rb_sink_cons) = create_sink();

        let reset_listeners = sink.reset_listeners();

        let faucets = Arc::new(RwLock::new(HashMap::new()));
        let duplicator = Duplicator {
            reset_listeners: sink.reset_listeners(),
            sink: Some(sink),
            faucets: faucets.clone(),
        };

        smol::spawn(async move {
            let mut ct = reset_listeners.create_cancellation_token();
            loop {
                if ct.is_canceled() {
                    rb_sink_cons.clear();
                    ct = reset_listeners.create_cancellation_token();
                }

                let next_sample = match rb_sink_cons.next().await {
                    Some(next_sample) => next_sample,
                    None => Err(FaucetError::UnknownError),
                };

                let faucets_read = faucets.read().unwrap().clone();
                let faucets = faucets_read.values().clone();

                if faucets.len() == 0 {
                    // TODO: better way to do this
                    smol::Timer::after(Duration::from_millis(100)).await;
                } else {
                    for faucet_buffer in faucets {
                        if faucet_buffer.send(next_sample.clone()).await.is_err() {
                            warn!("Faucet buffer is closed");

                            // TODO: ?
                        }
                    }
                }
            }
        })
        .detach();

        duplicator
    }

    pub fn open_faucet(&mut self) -> Faucet {
        let (faucet, mut rb_faucet_prod, reset_faucet) = create_faucet();

        let reset_listeners = self.reset_listeners.clone();
        reset_listeners.add_reset_listener(reset_faucet);

        let id = random();
        let (tx, rx) = async_channel::bounded(SAMPLE_BUFFER_SIZE);

        let faucets = self.faucets.clone();
        faucets.write().unwrap().insert(id, tx);

        smol::spawn(async move {
            let mut ct = reset_listeners.create_cancellation_token();
            loop {
                if ct.is_canceled() {
                    while !rx.is_empty() {
                        rx.recv().await.unwrap().unwrap();
                    }
                    ct = reset_listeners.create_cancellation_token();
                }
                let Ok(sample) = rx.recv().await else {
                    break;
                };
                let Ok(_) = rb_faucet_prod.push(sample).await else {
                    break;
                };
            }

            faucets.write().unwrap().remove(&id);
        })
        .detach();
        faucet
    }

    pub fn sink(&mut self) -> Sink {
        self.sink
            .take()
            .expect("RubatoResampler: tried to take sink twice")
    }
}
