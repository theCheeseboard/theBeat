use async_ringbuf::traits::Split;
use crate::audio_processing::audio_metadata::AudioMetadata;
use crate::audio_processing::audio_pipeline::{PipelineSample, SAMPLE_BUFFER_SIZE};
use async_ringbuf::traits::AsyncProducer;
use async_ringbuf::{AsyncHeapCons, AsyncHeapProd, AsyncHeapRb};
use std::sync::{Arc, RwLock};
use async_ringbuf::wrap::AsyncCons;
use smol::stream::StreamExt;

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

pub fn create_sink() -> (Sink, AsyncHeapCons<PipelineSample>) {
    let (rb_sink_prod, rb_sink_cons) =
        AsyncHeapRb::<PipelineSample>::new(SAMPLE_BUFFER_SIZE).split();
    let sink = Sink::new(rb_sink_prod);
    
    (sink, rb_sink_cons)
}

pub fn create_dummy_sink() -> Sink {
    let samples_buffer = AsyncHeapRb::<PipelineSample>::new(SAMPLE_BUFFER_SIZE);
    let (samples_producer, mut samples_consumer) = samples_buffer.split();
    smol::spawn(async move {
        loop {
            let next = samples_consumer.next().await;
            if next.is_none() {
                return;
            }
        }
    }).detach();
    Sink::new(samples_producer)
}