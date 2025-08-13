use crate::audio_processing::audio_metadata::AudioMetadata;
use crate::audio_processing::audio_pipeline::PipelineSample;
use async_ringbuf::traits::AsyncProducer;
use async_ringbuf::AsyncHeapProd;
use std::sync::{Arc, RwLock};

pub struct Sink {
    rb_producer: AsyncHeapProd<PipelineSample>,
    metadata: Arc<RwLock<AudioMetadata>>,
}

impl Sink {
    pub fn new(consumer: AsyncHeapProd<PipelineSample>) -> Self {
        Sink {
            rb_producer: consumer,
            metadata: Default::default(),
        }
    }

    pub async fn push_sample(&mut self, sample: PipelineSample) -> Result<(), PipelineSample> {
        self.rb_producer.push(sample).await
    }

    pub fn update_metadata(&mut self, metadata: Arc<RwLock<AudioMetadata>>) {
        self.metadata = metadata;
    }
}
