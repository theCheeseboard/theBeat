use crate::audio_processing::audio_pipeline::Faucet;
use crate::audio_processing::audio_pipeline::PipelineSampleResult;
use crate::audio_processing::audio_pipeline::Sink;
use crate::audio_processing::audio_pipeline::faucet::create_faucet;
use crate::audio_processing::audio_pipeline::sink::{ResetListenerGroupTrait, create_sink};
use async_ringbuf::AsyncHeapRb;
use async_ringbuf::traits::AsyncProducer;
use async_ringbuf::traits::Split;
use smol::stream::StreamExt;

pub fn create_bufferer(buffer_size: usize) -> (Sink, Faucet) {
    let (faucet, mut rb_faucet_prod, reset_faucet) = create_faucet();
    let (sink, mut rb_sink_cons) = create_sink();

    let (mut buffer_prod, mut buffer_cons) =
        AsyncHeapRb::<PipelineSampleResult>::new(buffer_size).split();

    let reset_listeners = sink.reset_listeners();
    reset_listeners.add_reset_listener(Box::new(move |_| reset_faucet()));

    smol::spawn(async move {
        loop {
            let next = rb_sink_cons.next().await.unwrap();
            buffer_prod.push(next).await.unwrap();
        }
    })
    .detach();
    smol::spawn(async move {
        loop {
            let next = buffer_cons.next().await.unwrap();
            rb_faucet_prod.push(next).await.unwrap();
        }
    })
    .detach();

    (sink, faucet)
}
