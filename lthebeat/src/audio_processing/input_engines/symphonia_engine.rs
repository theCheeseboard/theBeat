mod http_source;

use crate::audio_processing::audio_metadata::{Art, AudioMetadata};
use crate::audio_processing::audio_pipeline::faucet::{Faucet, FaucetError, create_faucet};
use crate::audio_processing::audio_pipeline::{
    PipelineSample, PipelineSampleResult, SAMPLE_BUFFER_SIZE,
};
use crate::audio_processing::input_engines::Controller;
use crate::audio_processing::input_engines::symphonia_engine::http_source::HttpSource;
use crate::audio_processing::sample::{Sample, SampleData};
use crate::play_queue::media_item::MediaItem;
use async_ringbuf::AsyncHeapRb;
use async_ringbuf::traits::{AsyncProducer, Split};
use gpui::Entity;
use log::warn;
use regex::Regex;
use std::borrow::Cow;
use std::fs::File;
use std::sync::{Arc, RwLock};
use std::thread;
use std::time::{Duration, Instant};
use symphonia::core::audio::{AudioBuffer, AudioBufferRef, Signal};
use symphonia::core::formats::{FormatReader, SeekMode, SeekTo};
use symphonia::core::io::{MediaSource, MediaSourceStream, MediaSourceStreamOptions};
use symphonia::core::meta::{
    MetadataLog, MetadataRevision, StandardTagKey, StandardVisualKey, Value,
};
use symphonia::core::probe::Hint;
use symphonia::core::units::Time;
use symphonia::default::{get_codecs, get_probe};
use tracing::info;
use tracing_subscriber::fmt::time;
use url::Url;

pub struct SymphoniaEngine {
    faucet: Option<Faucet>,
    format: Arc<RwLock<Box<dyn FormatReader>>>,
}

impl SymphoniaEngine {
    pub fn new(url: Url, associated_track: Option<Entity<MediaItem>>) -> anyhow::Result<Self> {
        let (music, hint) = open_media_source(&url)?;

        let stream_metadata_log = music.metadata_log();

        let media_source_stream =
            MediaSourceStream::new(music, MediaSourceStreamOptions::default());
        let meta_opts = Default::default();
        let format_opts = Default::default();
        let probe = get_probe();
        let mut probe_result =
            probe.format(&hint, media_source_stream, &format_opts, &meta_opts)?;

        let codec_registry = get_codecs();
        let first_track = probe_result.format.default_track().unwrap();
        let decoder_opts = Default::default();
        let mut decoder = codec_registry.make(&first_track.codec_params, &decoder_opts)?;

        let time_base = first_track.codec_params.time_base;
        let track_duration = time_base.and_then(|time_base| {
            first_track.codec_params.n_frames.map(|n_frames| {
                let time = time_base.calc_time(n_frames);
                Duration::from_secs(time.seconds) + Duration::from_secs_f64(time.frac)
            })
        });

        let format = Arc::new(RwLock::new(probe_result.format));
        let format_clone = format.clone();

        let (faucet, mut rb_prod, _) = create_faucet();
        let faucet_epoch = faucet.current_epoch();
        thread::spawn(move || {
            let mut start_instant = None;
            let mut file_meta = AudioMetadata {
                url: Some(url),
                duration: track_duration,
                associated_item: associated_track.clone(),
                ..AudioMetadata::default()
            };
            if let Some(probe_meta) = probe_result
                .metadata
                .get()
                .as_ref()
                .and_then(|m| m.current())
            {
                populate_metadata(&mut file_meta, probe_meta);
            }
            if let Some(next_meta) = stream_metadata_log.write().unwrap().metadata().current() {
                populate_metadata(&mut file_meta, next_meta);
            }
            let mut format_write = format.write().unwrap();
            if let Some(next_meta) = format_write.metadata().current() {
                populate_metadata(&mut file_meta, next_meta);
            }
            drop(format_write);

            let mut pushed_first_sample = false;
            loop {
                let mut format_write = format.write().unwrap();
                let next_sample = {
                    let next_packet = match format_write.next_packet() {
                        Ok(packet) => packet,
                        Err(err) => {
                            warn!("SymphoniaEngine: error while reading packet: {err}");
                            smol::block_on(rb_prod.push(Err(FaucetError::UnknownError))).unwrap();
                            return;
                        }
                    };

                    let mut stream_metadata = stream_metadata_log.write().unwrap();
                    while !stream_metadata.metadata().is_latest() {
                        stream_metadata.metadata().pop();
                        if let Some(next_meta) = stream_metadata.metadata().current() {
                            populate_metadata(&mut file_meta, next_meta);
                        }
                    }
                    drop(stream_metadata);

                    let mut meta = format_write.metadata();
                    while !meta.is_latest() {
                        meta.pop();
                        if let Some(next_meta) = meta.current() {
                            populate_metadata(&mut file_meta, next_meta);
                        }
                    }
                    drop(format_write);

                    let decoded = decoder.decode(&next_packet).unwrap();

                    let rate = decoded.spec().rate;
                    let channel_count = decoded.spec().channels.count();

                    let sample_data = match decoded {
                        AudioBufferRef::U8(v) => SampleData::Unsigned8(create_sample_data(v)),
                        AudioBufferRef::U16(v) => SampleData::Unsigned16(create_sample_data(v)),
                        AudioBufferRef::U32(v) => SampleData::Unsigned32(create_sample_data(v)),
                        AudioBufferRef::S8(v) => SampleData::Signed8(create_sample_data(v)),
                        AudioBufferRef::S16(v) => SampleData::Signed16(create_sample_data(v)),
                        AudioBufferRef::S32(v) => SampleData::Signed32(create_sample_data(v)),
                        AudioBufferRef::F32(v) => SampleData::Float32(create_sample_data(v)),
                        AudioBufferRef::F64(v) => SampleData::Float64(create_sample_data(v)),
                        _ => panic!("SymphoniaEngine: unsupported sample format"),
                    };

                    let elapsed = time_base
                        .map(|time_base| {
                            let time = time_base.calc_time(next_packet.ts);
                            Duration::from_secs(time.seconds) + Duration::from_secs_f64(time.frac)
                        })
                        .or_else(|| start_instant.map(|instant: Instant| instant.elapsed()));

                    let faucet_epoch = *faucet_epoch.read().unwrap();
                    if !pushed_first_sample {
                        if smol::block_on(rb_prod.push(Ok(PipelineSample::Sample(Sample::new(
                            rate,
                            channel_count as u16,
                            file_meta.clone(),
                            None,
                            elapsed,
                            SampleData::Empty,
                            associated_track.clone(),
                            faucet_epoch,
                        )))))
                        .is_err()
                        {
                            warn!("SymphoniaEngine: error while pushing sample to buffer");
                            warn!("SymphoniaEngine: stopping");
                            return;
                        }
                        pushed_first_sample = true;
                        start_instant = Some(Instant::now());
                    }

                    Some(Sample::new(
                        rate,
                        channel_count as u16,
                        file_meta.clone(),
                        None,
                        elapsed,
                        sample_data,
                        associated_track.clone(),
                        faucet_epoch,
                    ))
                };

                if next_sample.is_none() {
                    smol::block_on(rb_prod.push(Err(FaucetError::EndOfStream))).unwrap();
                    return;
                }

                if smol::block_on(
                    rb_prod.push(
                        next_sample
                            .ok_or(FaucetError::UnknownError)
                            .map(PipelineSample::Sample),
                    ),
                )
                .is_err()
                {
                    warn!("SymphoniaEngine: error while pushing sample to buffer");
                    warn!("SymphoniaEngine: stopping");
                    return;
                }
            }
        });

        Ok(Self {
            faucet: Some(faucet),
            format: format_clone,
        })
    }

    pub async fn audio_metadata(url: Url) -> anyhow::Result<AudioMetadata> {
        let (music, hint) = open_media_source(&url)?;

        let media_source_stream =
            MediaSourceStream::new(music, MediaSourceStreamOptions::default());
        let meta_opts = Default::default();
        let format_opts = Default::default();
        let probe = get_probe();
        let mut probe_result =
            probe.format(&hint, media_source_stream, &format_opts, &meta_opts)?;

        let mut format = probe_result.format;
        let first_track = format.default_track().unwrap();

        let time_base = first_track.codec_params.time_base;
        let track_duration = time_base.and_then(|time_base| {
            first_track.codec_params.n_frames.map(|n_frames| {
                let time = time_base.calc_time(n_frames);
                Duration::from_secs(time.seconds) + Duration::from_secs_f64(time.frac)
            })
        });

        let mut file_meta = AudioMetadata {
            url: Some(url),
            duration: track_duration,
            ..AudioMetadata::default()
        };
        if let Some(probe_meta) = probe_result
            .metadata
            .get()
            .as_ref()
            .and_then(|m| m.current())
        {
            populate_metadata(&mut file_meta, probe_meta);
        }
        if let Some(next_meta) = format.metadata().current() {
            populate_metadata(&mut file_meta, next_meta);
        }

        Ok(file_meta)
    }
}

impl Controller for SymphoniaEngine {
    fn faucet(&mut self) -> Faucet {
        self.faucet
            .take()
            .expect("SymphoniaEngine: tried to take faucet twice")
    }

    fn seek(&mut self, position: Duration) {
        let mut format_write = self.format.write().unwrap();
        format_write
            .seek(
                SeekMode::Accurate,
                SeekTo::Time {
                    time: position.into(),
                    track_id: None,
                },
            )
            .unwrap();
    }
}

fn open_media_source(url: &Url) -> anyhow::Result<(Box<dyn ExternalMetadata>, Hint)> {
    match url.scheme() {
        "file" => {
            let path = url
                .to_file_path()
                .map_err(|_| anyhow::anyhow!("Unable to decode file path from URL"))?;
            let mut hint = Hint::new();
            if let Some(extension) = path.extension() {
                hint.with_extension(extension.to_str().unwrap());
            }
            Ok((
                Box::new(File::open(path)?) as Box<dyn ExternalMetadata>,
                hint,
            ))
        }
        "http" | "https" => Ok((
            Box::new(HttpSource::new(url.clone())) as Box<dyn ExternalMetadata>,
            Hint::new(),
        )),
        _ => Err(anyhow::anyhow!("Unsupported scheme")),
    }
}

fn create_sample_data<T>(v: Cow<AudioBuffer<T>>) -> Vec<T>
where
    T: symphonia::core::sample::Sample,
{
    let channel_count = v.spec().channels.count();
    let mut samples: Vec<T> = vec![T::default(); channel_count * v.capacity()];

    for i in 0..channel_count {
        let chan = v.chan(i);
        for sample_i in 0..chan.len() {
            samples[sample_i * channel_count + i] = chan[sample_i];
        }
    }

    samples
}

fn populate_metadata(metadata: &mut AudioMetadata, symphonia_metadata: &MetadataRevision) {
    let id3_position_in_set_regex = Regex::new(r"(\d+)/(\d+)").unwrap();

    for tag in symphonia_metadata.tags() {
        match tag.std_key {
            Some(StandardTagKey::TrackTitle) => metadata.title = Some(tag.value.to_string()),
            Some(StandardTagKey::Artist) => metadata.artist = Some(tag.value.to_string()),
            Some(StandardTagKey::Album) => metadata.album = Some(tag.value.to_string()),
            Some(StandardTagKey::TrackNumber) => match &tag.value {
                Value::String(v) => {
                    if let Some(captures) = id3_position_in_set_regex.captures(v) {
                        if let Some(track) = captures.get(1) {
                            metadata.track_number = track.as_str().parse().ok()
                        }
                        if let Some(total) = captures.get(2) {
                            metadata.total_track_number = total.as_str().parse().ok();
                        }
                    } else {
                        metadata.track_number = v.clone().parse().ok();
                    }
                }
                Value::UnsignedInt(v) => {
                    metadata.track_number = Some(*v as u32);
                }
                _ => (),
            },
            Some(StandardTagKey::DiscNumber) => match &tag.value {
                Value::String(v) => {
                    if let Some(captures) = id3_position_in_set_regex.captures(v) {
                        if let Some(track) = captures.get(1) {
                            metadata.disc_number = track.as_str().parse().ok()
                        }
                        if let Some(total) = captures.get(2) {
                            metadata.total_disc_number = total.as_str().parse().ok();
                        }
                    } else {
                        metadata.track_number = v.clone().parse().ok();
                    }
                }
                Value::UnsignedInt(v) => {
                    metadata.track_number = Some(*v as u32);
                }
                _ => (),
            },
            _ => {}
        }
    }

    if !symphonia_metadata.visuals().is_empty() {
        let album_cover = symphonia_metadata
            .visuals()
            .iter()
            .find(|visual| visual.usage == Some(StandardVisualKey::FrontCover))
            .unwrap_or_else(|| symphonia_metadata.visuals().first().unwrap());
        metadata.album_cover = Some(Arc::new(Art::new(
            album_cover.data.clone(),
            album_cover.media_type.clone(),
        )));
    }
}

trait ExternalMetadata: MediaSource {
    fn metadata_log(&self) -> Arc<RwLock<MetadataLog>> {
        Arc::new(RwLock::new(Default::default()))
    }
}

impl ExternalMetadata for File {}
