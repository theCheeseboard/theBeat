use crate::audio_processing::audio_metadata::AudioMetadata;
use crate::audio_processing::audio_pipeline::{PipelineSample, SAMPLE_BUFFER_SIZE};
use async_ringbuf::traits::{Observer, Split};
use async_ringbuf::{AsyncHeapCons, AsyncHeapProd, AsyncHeapRb};
use smol::stream::StreamExt;
use std::sync::{Arc, RwLock};

pub struct Faucet {
    rb_consumer: AsyncHeapCons<PipelineSample>,
    metadata: Arc<RwLock<AudioMetadata>>
}

#[derive(Debug, PartialEq, Clone, Copy)]
pub enum FaucetError {
    UnknownError,
    EndOfStream
}

impl Faucet {
    pub fn new(consumer: AsyncHeapCons<PipelineSample>) -> Self {
        Faucet {
            rb_consumer: consumer,
            metadata: Default::default()
        }
    }

    pub async fn next_sample(&mut self) -> PipelineSample {
        self.rb_consumer.next().await.ok_or(FaucetError::UnknownError)?
    }
    
    pub fn samples_waiting(&self) -> usize {
        self.rb_consumer.occupied_len()
    }
}

pub fn create_faucet() -> (Faucet, AsyncHeapProd<PipelineSample>) {
    let (rb_faucet_prod, rb_faucet_cons) =
        AsyncHeapRb::<PipelineSample>::new(SAMPLE_BUFFER_SIZE).split();
    let sink = Faucet::new(rb_faucet_cons);

    (sink, rb_faucet_prod)
}