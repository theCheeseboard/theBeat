use crate::audio_processing::audio_metadata::AudioMetadata;
use crate::audio_processing::audio_pipeline::resetter::Resetter;
use crate::audio_processing::audio_pipeline::{
    PipelineSampleResult, ResetPipelineFunction, SAMPLE_BUFFER_SIZE,
};
use async_ringbuf::traits::Split;
use async_ringbuf::traits::{AsyncProducer, Based, Observer, Producer};
use async_ringbuf::{AsyncHeapCons, AsyncHeapProd, AsyncHeapRb};
use cancellation_token::{CancellationToken, CancellationTokenSource};
use smol::stream::StreamExt;
use std::sync::{Arc, RwLock};

pub type ResetListenerGroup = Arc<RwLock<Vec<ResetPipelineFunction>>>;

pub struct Sink {
    rb_producer: AsyncHeapProd<PipelineSampleResult>,
    metadata: Arc<RwLock<AudioMetadata>>,
    resetter: Option<Resetter>,
    reset_listeners: ResetListenerGroup,
}

impl Sink {
    pub fn new(consumer: AsyncHeapProd<PipelineSampleResult>) -> Self {
        Sink {
            rb_producer: consumer,
            metadata: Default::default(),
            resetter: None,
            reset_listeners: Default::default(),
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

    pub fn set_resetter(&mut self, resetter: Option<Resetter>) {
        self.resetter = resetter;
        if let Some(resetter) = &self.resetter {
            resetter.add_reset_listener_group(self.reset_listeners.clone());
        }
    }

    pub fn add_reset_listener(&self, listener: ResetPipelineFunction) {
        self.reset_listeners.add_reset_listener(listener);
    }

    pub fn reset_listeners(&self) -> ResetListenerGroup {
        self.reset_listeners.clone()
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

pub trait ResetListenerGroupTrait {
    fn add_reset_listener(&self, listener: ResetPipelineFunction);
    fn create_cancellation_token(&self) -> CancellationToken;
}

impl ResetListenerGroupTrait for Arc<RwLock<Vec<ResetPipelineFunction>>> {
    fn add_reset_listener(&self, listener: ResetPipelineFunction) {
        self.write().unwrap().push(listener);
    }

    fn create_cancellation_token(&self) -> CancellationToken {
        let cancellation_token_source = CancellationTokenSource::new();
        let cancellation_token = cancellation_token_source.token();

        self.add_reset_listener(Box::new(move |_| {
            cancellation_token_source.cancel();
        }));

        cancellation_token
    }
}
