use crate::audio_processing::audio_metadata::AudioMetadata;
use crate::audio_processing::audio_pipeline::{PipelineSampleResult, SAMPLE_BUFFER_SIZE};
use async_ringbuf::traits::AsyncProducer;
use async_ringbuf::traits::Split;
use async_ringbuf::{AsyncHeapCons, AsyncHeapProd, AsyncHeapRb};
use smol::stream::StreamExt;
use std::sync::{Arc, RwLock};

pub struct Sink {
    rb_producer: AsyncHeapProd<PipelineSampleResult>,
    metadata: Arc<RwLock<AudioMetadata>>,
}

impl Sink {
    pub fn new(consumer: AsyncHeapProd<PipelineSampleResult>) -> Self {
        Sink {
            rb_producer: consumer,
            metadata: Default::default(),
        }
    }

    pub async fn push_sample(
        &mut self,
        sample: PipelineSampleResult,
    ) -> Result<(), PipelineSampleResult> {
        self.rb_producer.push(sample).await
    }

    pub fn update_metadata(&mut self, metadata: Arc<RwLock<AudioMetadata>>) {
        self.metadata = metadata;
    }
}

pub fn create_sink() -> (Sink, AsyncHeapCons<PipelineSampleResult>) {
    let (rb_sink_prod, rb_sink_cons) =
        AsyncHeapRb::<PipelineSampleResult>::new(SAMPLE_BUFFER_SIZE).split();
    let sink = Sink::new(rb_sink_prod);

    (sink, rb_sink_cons)
}

pub fn create_dummy_sink() -> Sink {
    let samples_buffer = AsyncHeapRb::<PipelineSampleResult>::new(SAMPLE_BUFFER_SIZE);
    let (samples_producer, mut samples_consumer) = samples_buffer.split();
    smol::spawn(async move {
        loop {
            let next = samples_consumer.next().await;
            if next.is_none() {
                return;
            }
        }
    })
        .detach();
    Sink::new(samples_producer)
}
