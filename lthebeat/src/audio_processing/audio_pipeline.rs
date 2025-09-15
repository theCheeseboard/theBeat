use crate::audio_processing::audio_pipeline::faucet::{Faucet, FaucetError};
use crate::audio_processing::audio_pipeline::sink::Sink;
use crate::audio_processing::sample::Sample;
use log::warn;
use std::sync::{Arc, RwLock};

pub mod attenuator;
pub mod audio_format;
pub mod duplicator;
pub mod faucet;
mod resetter;
pub mod sink;
pub mod sync_lock;
pub mod sync_lock_sync;

pub const SAMPLE_BUFFER_SIZE: usize = 4;

pub type PipelineSampleResult = Result<PipelineSample, FaucetError>;

pub type ResetPipelineFunction = Box<dyn Fn(u16) + Send + Sync>;

#[derive(Debug, Clone)]
pub enum PipelineSample {
    Sample(Sample),
}

struct Plug {
    faucet: Faucet,
    sink: Sink,
}

pub fn plug(mut faucet: Faucet, mut sink: Sink) {
    sink.set_resetter(Some(faucet.resetter()));
    smol::spawn(async move {
        loop {
            let next_sample = faucet.next_sample().await;
            if sink.push_sample(next_sample).await.is_err() {
                warn!("Failed to push sample to sink; closing plug");
                return;
            }
        }
    })
    .detach();
}

pub trait PipelineSampleExt {
    fn with_epoch(self, epoch: u16) -> Self;

    fn with_epoch_ref(self, epoch_ref: &Arc<RwLock<u16>>) -> Self
    where
        Self: Sized,
    {
        let epoch = *epoch_ref.read().unwrap();
        self.with_epoch(epoch)
    }
}

impl PipelineSampleExt for PipelineSampleResult {
    fn with_epoch(self, epoch: u16) -> Self {
        self.map(|s| s.with_epoch(epoch))
    }
}

impl PipelineSampleExt for PipelineSample {
    fn with_epoch(self, epoch: u16) -> Self {
        match self {
            PipelineSample::Sample(sample) => PipelineSample::Sample(sample.with_epoch(epoch)),
        }
    }
}

impl PipelineSampleExt for Sample {
    fn with_epoch(mut self, epoch: u16) -> Self {
        self.epoch = epoch;
        self
    }
}
