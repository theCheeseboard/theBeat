use log::warn;
use crate::audio_processing::audio_pipeline::faucet::{Faucet, FaucetError};
use crate::audio_processing::audio_pipeline::sink::Sink;
use crate::audio_processing::sample::Sample;

pub mod audio_format;
pub mod faucet;
pub mod sink;
pub mod duplicator;
pub mod sync_lock;
pub mod sync_lock_sync;

pub const SAMPLE_BUFFER_SIZE: usize = 4;

pub type PipelineSampleResult = Result<PipelineSample, FaucetError>;

#[derive(Debug, Clone)]
pub enum PipelineSample {
    Sample(Sample),
    Reset
}

struct Plug {
    faucet: Faucet,
    sink: Sink,
}

pub fn plug(mut faucet: Faucet, mut sink: Sink) {
    smol::spawn(async move {
        loop {
            let next_sample = faucet.next_sample().await;
            if sink.push_sample(next_sample)
                .await.is_err() {
                warn!("Failed to push sample to sink; closing plug");
                return;
            }
        }
    })
    .detach();
}
