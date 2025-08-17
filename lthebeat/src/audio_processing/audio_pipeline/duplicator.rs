use crate::audio_processing::audio_pipeline::audio_format::SampleFormat;
use crate::audio_processing::audio_pipeline::faucet::{Faucet, FaucetError};
use crate::audio_processing::audio_pipeline::sink::Sink;
use crate::audio_processing::audio_pipeline::{PipelineSample, SAMPLE_BUFFER_SIZE};
use crate::audio_processing::sample::{Sample, SampleData};
use async_ringbuf::traits::{AsyncProducer, Split};
use async_ringbuf::{AsyncHeapProd, AsyncHeapRb};
use smol::stream::StreamExt;
use std::cell::RefCell;
use std::collections::{HashMap, VecDeque};
use std::rc::Rc;
use std::sync::{Arc, RwLock};
use std::time::Duration;
use async_channel::Sender;
use log::warn;
use rand::random;

pub struct Duplicator {
    sink: Option<Sink>,
    faucets: Arc<RwLock<HashMap<u64, Sender<PipelineSample>>>>,
}

impl Default for Duplicator {
    fn default() -> Self {
        Self::new()
    }
}

impl Duplicator {
    pub fn new() -> Self {
        let (rb_sink_prod, mut rb_sink_cons) =
            AsyncHeapRb::<PipelineSample>::new(SAMPLE_BUFFER_SIZE).split();

        let faucets = Arc::new(RwLock::new(HashMap::new()));
        let duplicator = Duplicator {
            sink: Some(Sink::new(rb_sink_prod)),
            faucets: faucets.clone(),
        };

        smol::spawn(async move {
            loop {
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
        let (mut rb_faucet_prod, rb_faucet_cons) =
            AsyncHeapRb::<PipelineSample>::new(SAMPLE_BUFFER_SIZE).split();

        let id = random();
        let (tx, rx) = async_channel::bounded(SAMPLE_BUFFER_SIZE);

        let faucets = self.faucets.clone();
        faucets.write().unwrap().insert(id, tx);

        smol::spawn(async move {
            loop {
                let Ok(sample) = rx.recv().await else {
                    break;
                };
                let Ok(_) = rb_faucet_prod.push(sample).await else {
                    break;
                };
            }

            faucets.write().unwrap().remove(&id);
        }).detach();
        Faucet::new(rb_faucet_cons)
    }

    pub fn sink(&mut self) -> Sink {
        self.sink
            .take()
            .expect("RubatoResampler: tried to take sink twice")
    }
}
