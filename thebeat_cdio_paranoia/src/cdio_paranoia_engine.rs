use async_ringbuf::traits::Split;
use async_ringbuf::traits::{AsyncProducer, Consumer};
use std::time::Duration;
pub mod cdio_manager;
mod cdio_paranoia;
mod cdio_paranoia_track;
mod lsn;

use async_ringbuf::AsyncHeapRb;
use gpui::{App, Entity};
use libcdio_sys::{CdIo_t, cdio_open_cd};
use lsn::Lsn;
use lthebeat::audio_processing::audio_metadata::AudioMetadata;
use lthebeat::audio_processing::audio_pipeline::bufferer::create_bufferer;
use lthebeat::audio_processing::audio_pipeline::faucet::{Faucet, FaucetError, create_faucet};
use lthebeat::audio_processing::audio_pipeline::{PipelineSample, PipelineSampleResult, plug};
use lthebeat::audio_processing::input_engines::{Controller, EngineFactory};
use lthebeat::audio_processing::sample::Sample;
use lthebeat::play_queue::media_item::MediaItem;
use smol::stream::StreamExt;
use std::ffi::CString;
use std::fs::File;
use std::rc::Rc;
use std::sync::{Arc, RwLock};
use std::thread;
use std::time::Instant;
use tracing::{info, warn};
use url::Url;
use crate::cdio_paranoia_engine::cdio_manager::{CdioCd, CdioManager};
use crate::cdio_paranoia_engine::cdio_paranoia::CdioParanoia;

pub struct CdioParanoiaEngine {
    faucet: Option<Faucet>,
    next_requested_lsn: Arc<RwLock<Lsn>>,
    reset_faucet: Box<dyn Fn() + Send + Sync>,
}

#[derive(Default)]
pub struct CdioParanoiaFactory {

}

impl EngineFactory for CdioParanoiaFactory {
    fn faucet_for_url(&self, url: Url, associated_track: Option<Entity<MediaItem>>, cx: &mut App) -> Option<Box<dyn Controller>> {
        if let Ok(cdio_paranoia_engine) = CdioParanoiaEngine::new(url.clone(), associated_track.clone(), cx) {
            Some(Box::new(cdio_paranoia_engine))
        } else {
            None
        }
    }
}

impl CdioParanoiaEngine {
    pub fn new(
        url: Url,
        associated_track: Option<Entity<MediaItem>>,
        cx: &mut App,
    ) -> anyhow::Result<Self> {
        let (cdio_cd, requested_track) = open_media_source(&url, cx)?;

        let (faucet, mut rb_prod, reset_faucet) = create_faucet();
        let faucet_epoch = faucet.current_epoch();
        let faucet_epoch_clone = faucet_epoch.clone();

        let (mut buffer_prod, mut buffer_cons) =
            AsyncHeapRb::<PipelineSampleResult>::new(1500).split();

        info!("CD first track: {}", cdio_cd.first_track());
        info!("CD last track: {}", cdio_cd.last_track());
        for track in cdio_cd.first_track()..cdio_cd.last_track() {
            info!(
                "CD track {track} information: {:?}",
                cdio_cd.track_information(track)
            );
        }

        let last_track = cdio_cd.last_track();

        let paranoia = CdioCd::paranoia(cdio_cd);
        let next_requested_lsn = Arc::new(RwLock::new(Lsn(0)));
        let next_requested_lsn_clone = next_requested_lsn.clone();

        let track = CdioParanoia::get_track(paranoia.clone(), requested_track)?;

        let meta = AudioMetadata {
            url: Some(url.clone()),
            title: Some(format!("Track {}", requested_track)),
            duration: Some(track.max_len().into()),
            track_number: Some(requested_track.into()),
            total_track_number: Some(last_track.into()),
            ..AudioMetadata::default()
        };

        thread::spawn(move || {
            paranoia.init_if_required();

            loop {
                let next_lsn = *next_requested_lsn.read().unwrap();
                let x = Instant::now();
                let Some(frame) = track.read_relative_lsn(next_lsn) else {
                    smol::block_on(buffer_prod.push(Err(FaucetError::EndOfStream))).unwrap();
                    return;
                };
                let mut next_lsn_borrow = next_requested_lsn.write().unwrap();
                if *next_lsn_borrow != next_lsn {
                    // User requested a seek, discard this read
                    continue;
                }
                next_lsn_borrow.0 += 1;
                let elapsed = (*next_lsn_borrow).into();
                drop(next_lsn_borrow);

                let faucet_epoch = *faucet_epoch.read().unwrap();
                let next_sample = Sample::new(
                    44100,
                    2,
                    meta.clone(),
                    None,
                    Some(elapsed),
                    frame,
                    associated_track.clone(),
                    faucet_epoch,
                );

                if smol::block_on(buffer_prod.push(Ok(PipelineSample::Sample(next_sample))))
                    .is_err()
                {
                    warn!("error while pushing sample to buffer");
                    warn!("stopping");
                    return;
                }
            }
        });
        smol::spawn(async move {
            loop {
                let next = buffer_cons.next().await.unwrap();
                let faucet_epoch = *faucet_epoch_clone.read().unwrap();
                if let Ok(PipelineSample::Sample(sample)) = &next
                    && sample.epoch != faucet_epoch
                {
                    continue;
                }
                rb_prod.push(next).await.unwrap();
            }
        })
        .detach();

        Ok(Self {
            faucet: Some(faucet),
            next_requested_lsn: next_requested_lsn_clone,
            reset_faucet,
        })
    }
}

impl Controller for CdioParanoiaEngine {
    fn faucet(&mut self) -> Faucet {
        self.faucet.take().expect("tried to take faucet twice")
    }

    fn seek(&mut self, duration: Duration) {
        (self.reset_faucet)();
        let mut next_lsn_borrow = self.next_requested_lsn.write().unwrap();
        *next_lsn_borrow = duration.into();
    }
}

fn open_media_source(url: &Url, cx: &mut App) -> anyhow::Result<(Arc<CdioCd>, u8)> {
    match url.scheme() {
        "cd" => {
            let cdio_manager = cx.global::<CdioManager>();
            let track_number = url
                .query_pairs()
                .find(|(k, _)| k == "track")
                .map(|(_, v)| v.parse::<u8>().unwrap())
                .unwrap_or(1);
            Ok((cdio_manager.get_cd(url.path().trim_end())?, track_number))
        }
        _ => Err(anyhow::anyhow!("Unsupported scheme")),
    }
}
