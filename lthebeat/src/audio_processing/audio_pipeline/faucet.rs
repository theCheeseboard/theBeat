use crate::audio_processing::audio_pipeline::resetter::Resetter;
use crate::audio_processing::audio_pipeline::{
    PipelineSample, PipelineSampleResult, ResetPipelineFunction, SAMPLE_BUFFER_SIZE,
};
use async_ringbuf::traits::{Based, Consumer, Observer, Split};
use async_ringbuf::{AsyncHeapCons, AsyncHeapProd, AsyncHeapRb};
use smol::stream::StreamExt;

pub struct Faucet {
    rb_consumer: AsyncHeapCons<PipelineSampleResult>,
    resetter: Resetter,
}

#[derive(Debug, PartialEq, Clone, Copy)]
pub enum FaucetError {
    UnknownError,
    EndOfStream,
}

impl Faucet {
    pub fn new(consumer: AsyncHeapCons<PipelineSampleResult>) -> Self {
        Faucet {
            rb_consumer: consumer,
            resetter: Resetter::new(),
        }
    }

    pub async fn next_sample(&mut self) -> PipelineSampleResult {
        self.rb_consumer
            .next()
            .await
            .ok_or(FaucetError::UnknownError)?
    }

    pub fn resetter(&self) -> Resetter {
        self.resetter.clone()
    }

    pub fn samples_waiting(&self) -> usize {
        self.rb_consumer.occupied_len()
    }
}

pub fn create_faucet() -> (
    Faucet,
    AsyncHeapProd<PipelineSampleResult>,
    ResetPipelineFunction,
) {
    let (rb_faucet_prod, rb_faucet_cons) =
        AsyncHeapRb::<PipelineSampleResult>::new(SAMPLE_BUFFER_SIZE).split();
    let faucet = Faucet::new(rb_faucet_cons);

    let resetter = faucet.resetter();

    (
        faucet,
        rb_faucet_prod,
        Box::new(move || resetter.trigger_reset()),
    )
}
