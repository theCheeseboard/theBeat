use crate::audio_processing::audio_pipeline::resetter::Resetter;
use crate::audio_processing::audio_pipeline::{
    PipelineSample, PipelineSampleResult, ResetPipelineFunction, SAMPLE_BUFFER_SIZE,
};
use async_ringbuf::traits::{Based, Consumer, Observer, Split};
use async_ringbuf::{AsyncHeapCons, AsyncHeapProd, AsyncHeapRb};
use smol::stream::StreamExt;
use std::sync::{Arc, RwLock};

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
        loop {
            let sample = self
                .rb_consumer
                .next()
                .await
                .ok_or(FaucetError::UnknownError)?;
            let current_epoch = self.resetter.current_epoch();
            if let Ok(PipelineSample::Sample(sample)) = &sample
                && sample.epoch != current_epoch
            {
                continue;
            }

            return sample;
        }
    }

    pub fn resetter(&self) -> Resetter {
        self.resetter.clone()
    }

    pub fn samples_waiting(&self) -> usize {
        self.rb_consumer.occupied_len()
    }

    pub fn current_epoch(&self) -> Arc<RwLock<u16>> {
        self.resetter.current_epoch_ref()
    }
}

pub fn create_faucet() -> (
    Faucet,
    AsyncHeapProd<PipelineSampleResult>,
    Box<dyn Fn() + Send + Sync>,
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
