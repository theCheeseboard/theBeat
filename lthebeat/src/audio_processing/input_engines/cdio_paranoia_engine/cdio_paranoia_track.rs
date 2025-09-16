use crate::audio_processing::input_engines::cdio_paranoia_engine::cdio_manager::CdioCdTrack;
use crate::audio_processing::input_engines::cdio_paranoia_engine::cdio_paranoia::CdioParanoia;
use crate::audio_processing::input_engines::cdio_paranoia_engine::lsn::Lsn;
use crate::audio_processing::sample::SampleData;
use std::sync::Arc;
use std::sync::Mutex;

pub struct CdioParanoiaTrack {
    cdio_paranoia: Arc<CdioParanoia>,
    track_information: CdioCdTrack,
    buffer: Mutex<Vec<Option<SampleData>>>,
}

impl CdioParanoiaTrack {
    pub fn new(cdio_paranoia: Arc<CdioParanoia>, track: u8) -> anyhow::Result<Self> {
        let track_information = cdio_paranoia
            .cdio_cd
            .track_information(track)
            .ok_or_else(|| anyhow::anyhow!("Track {} not found", track))?;

        let len = track_information.last_lsn - track_information.first_lsn + Lsn(1);

        Ok(CdioParanoiaTrack {
            cdio_paranoia,
            track_information,
            buffer: Mutex::new(vec![None; len.0 as usize]),
        })
    }

    pub fn read_relative_lsn(&self, lsn: Lsn) -> Option<SampleData> {
        let mut buffer = self.buffer.lock().unwrap();
        if lsn.0 as usize > buffer.len() {
            return None;
        }

        let frame = &buffer[lsn.0 as usize];
        if let Some(frame) = frame {
            return Some(frame.clone());
        }

        // Read in up to 750 frames
        let mut read_length = 750;
        for i in 1..751 {
            match buffer.get(lsn.0 as usize + i) {
                Some(Some(_)) => {
                    read_length = i;
                    break;
                }
                None => {
                    read_length = i - 1;
                    break;
                }
                _ => {}
            }
        }

        let frames = self
            .cdio_paranoia
            .get_frames(self.track_information.first_lsn + lsn, read_length);
        for (i, frame) in frames.iter().enumerate() {
            buffer[lsn.0 as usize + i] = Some(frame.clone());
        }

        buffer[lsn.0 as usize].clone()
    }

    pub fn max_len(&self) -> Lsn {
        self.track_information.last_lsn - self.track_information.first_lsn
    }
}
