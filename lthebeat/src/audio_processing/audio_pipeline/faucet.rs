use crate::audio_processing::audio_metadata::AudioMetadata;
use crate::audio_processing::audio_pipeline::PipelineSample;
use async_ringbuf::AsyncHeapCons;
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
}
