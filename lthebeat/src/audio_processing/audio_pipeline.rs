use crate::audio_processing::audio_pipeline::faucet::{Faucet, FaucetError};
use crate::audio_processing::audio_pipeline::sink::Sink;
use crate::audio_processing::sample::Sample;
use log::info;

pub mod audio_format;
pub mod faucet;
pub mod sink;

pub const SAMPLE_BUFFER_SIZE: usize = 128;

pub type PipelineSample = Result<Sample, FaucetError>;

struct Plug {
    faucet: Faucet,
    sink: Sink,
}

pub fn plug(mut faucet: Faucet, mut sink: Sink) {
    smol::spawn(async move {
        loop {
            let next_sample = faucet.next_sample().await;
            sink.push_sample(next_sample)
                .await
                .expect("failed to push sample to sink");
        }
    })
    .detach();
}
