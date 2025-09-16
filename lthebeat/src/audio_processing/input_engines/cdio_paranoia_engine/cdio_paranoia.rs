use crate::audio_processing::input_engines::cdio_paranoia_engine::cdio_manager::CdioCd;
use crate::audio_processing::input_engines::cdio_paranoia_engine::cdio_paranoia_track::CdioParanoiaTrack;
use crate::audio_processing::input_engines::cdio_paranoia_engine::lsn::Lsn;
use crate::audio_processing::sample::SampleData;
use libcdio_sys::{cdio_cd_enums_CDIO_CD_FRAMESIZE_RAW, cdio_cddap_close_no_free_cdio, cdio_cddap_disc_firstsector, cdio_cddap_identify_cdio, cdio_cddap_open, cdio_cddap_speed_set, cdio_cddap_verbose_set, cdio_paranoia_free, cdio_paranoia_init, cdio_paranoia_modeset, cdio_paranoia_read, cdio_paranoia_seek, cdio_set_speed, cdrom_drive_t, cdrom_paranoia_t, lsn_t, paranoia_cdda_enums_t_CDDA_MESSAGE_PRINTIT, paranoia_mode_t_PARANOIA_MODE_FULL};
use std::collections::HashMap;
use std::iter::Map;
use std::ptr::null_mut;
use std::slice;
use std::sync::Arc;
use std::sync::Mutex;
use std::sync::Weak;

pub struct CdioParanoia {
    pub cdio_cd: Arc<CdioCd>,
    paranoia: Mutex<Option<ParanoiaSession>>,
    tracks: Mutex<HashMap<u8, Weak<CdioParanoiaTrack>>>,
}

struct ParanoiaSession {
    paranoia: *mut cdrom_paranoia_t,
    drive: *mut cdrom_drive_t,
    lsn: Lsn,
}

struct CdFrame {
    sample_data: SampleData,
    lsn: Lsn,
}

impl Default for CdFrame {
    fn default() -> Self {
        CdFrame {
            sample_data: SampleData::Empty,
            lsn: Default::default(),
        }
    }
}

impl CdioParanoia {
    pub fn new(cdio_cd: Arc<CdioCd>) -> anyhow::Result<Self> {
        Ok(Self {
            cdio_cd,
            paranoia: Mutex::new(None),
            tracks: Mutex::new(HashMap::new()),
        })
    }

    pub fn init_if_required(&self) {
        let mut paranoia = self.paranoia.lock().unwrap();
        if paranoia.is_none() {
            *paranoia = Some(unsafe {
                let drive = cdio_cddap_identify_cdio(
                    self.cdio_cd.as_ptr(),
                    paranoia_cdda_enums_t_CDDA_MESSAGE_PRINTIT as i32,
                    null_mut(),
                );
                cdio_cddap_open(drive);
                cdio_cddap_verbose_set(
                    drive,
                    paranoia_cdda_enums_t_CDDA_MESSAGE_PRINTIT as i32,
                    paranoia_cdda_enums_t_CDDA_MESSAGE_PRINTIT as i32,
                );
                cdio_cddap_speed_set(drive, 12);

                let first_sector = cdio_cddap_disc_firstsector(drive);

                let paranoia = cdio_paranoia_init(drive);
                cdio_paranoia_modeset(paranoia, paranoia_mode_t_PARANOIA_MODE_FULL as i32);
                cdio_paranoia_seek(paranoia, first_sector, 0);

                ParanoiaSession {
                    paranoia,
                    drive,
                    lsn: Lsn(first_sector),
                }
            });
        }
    }

    pub fn get_frames(&self, lsn: Lsn, count: usize) -> Vec<SampleData> {
        let mut paranoia_lock = self.paranoia.lock().unwrap();
        let paranoia = paranoia_lock.as_mut().unwrap();

        if paranoia.lsn != lsn {
            unsafe {
                cdio_paranoia_seek(paranoia.paranoia, lsn.0, 0);
            }
            paranoia.lsn = lsn;
        }

        let mut frames = Vec::with_capacity(count);
        for _ in 0..count {
            unsafe {
                // buf always contains CDIO_CD_FRAMESIZE_RAW bytes
                let buf = cdio_paranoia_read(paranoia.paranoia, None);
                let next_frame = SampleData::Signed16(
                    slice::from_raw_parts(
                        buf,
                        cdio_cd_enums_CDIO_CD_FRAMESIZE_RAW as usize / size_of::<i16>(),
                    )
                    .to_vec(),
                );
                frames.push(next_frame);
                paranoia.lsn.0 += 1;
            }
        }
        frames
    }

    pub fn get_track(
        paranoia: Arc<CdioParanoia>,
        track_number: u8,
    ) -> anyhow::Result<Arc<CdioParanoiaTrack>> {
        let mut tracks = paranoia.tracks.lock().unwrap();
        if let Some(track) = tracks.get(&track_number).and_then(|weak| weak.upgrade()) {
            Ok(track)
        } else {
            let track = Arc::new(CdioParanoiaTrack::new(paranoia.clone(), track_number)?);
            tracks.insert(track_number, Arc::downgrade(&track));
            Ok(track)
        }
    }
}

unsafe impl Send for CdioParanoia {}
unsafe impl Sync for CdioParanoia {}

impl Drop for CdioParanoia {
    fn drop(&mut self) {
        unsafe {
            if let Some(paranoia) = self.paranoia.lock().unwrap().as_ref() {
                cdio_paranoia_free(paranoia.paranoia);

                // SAFETY: The cdio_cd is not owned by us, and will be dropped by the Arc<CdioCd> when necessary.
                cdio_cddap_close_no_free_cdio(paranoia.drive);
            }
        }
    }
}
