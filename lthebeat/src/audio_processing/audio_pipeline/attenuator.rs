use crate::audio_processing::audio_pipeline::PipelineSample;
use crate::audio_processing::audio_pipeline::faucet::{Faucet, create_faucet};
use crate::audio_processing::audio_pipeline::sink::{ResetListenerGroupTrait, Sink, create_sink};
use async_ringbuf::traits::AsyncProducer;
use smol::stream::StreamExt;
use std::sync::{Arc, RwLock};

pub struct Attenuator {
    sink: Option<Sink>,
    faucet: Option<Faucet>,
    attenuation_factor: Arc<RwLock<f64>>,
}

impl Attenuator {
    pub fn new(attenuation_factor: f64) -> Self {
        let (faucet, mut rb_faucet_prod, reset_faucet) = create_faucet();
        let (sink, mut rb_sink_cons) = create_sink();

        let reset_listeners = sink.reset_listeners();
        reset_listeners.add_reset_listener(Box::new(move |_| reset_faucet()));

        let attenuation_factor = Arc::new(RwLock::new(attenuation_factor));
        let attenuation_factor_clone = attenuation_factor.clone();

        smol::spawn(async move {
            loop {
                let Some(next_sample) = rb_sink_cons.next().await else {
                    return;
                };

                if let Ok(PipelineSample::Sample(next_sample)) = next_sample {
                    let attenuation_factor = *attenuation_factor.read().unwrap();

                    if rb_faucet_prod
                        .push(Ok(PipelineSample::Sample(
                            next_sample.attenuate(attenuation_factor),
                        )))
                        .await
                        .is_err()
                    {
                        return;
                    }
                } else if rb_faucet_prod.push(next_sample).await.is_err() {
                    return;
                }
            }
        })
        .detach();

        Self {
            sink: Some(sink),
            faucet: Some(faucet),
            attenuation_factor: attenuation_factor_clone,
        }
    }

    pub fn sink(&mut self) -> Sink {
        self.sink
            .take()
            .expect("Attenuator: tried to take sink twice")
    }

    pub fn faucet(&mut self) -> Faucet {
        self.faucet
            .take()
            .expect("Attenuator: tried to take faucet twice")
    }

    pub fn set_attenuation_factor(&self, attenuation_factor: f64) {
        *self.attenuation_factor.write().unwrap() = attenuation_factor;
    }
}
