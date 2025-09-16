use crate::audio_processing::input_engines::cdio_paranoia_engine::cdio_paranoia::CdioParanoia;
use crate::audio_processing::input_engines::cdio_paranoia_engine::lsn::Lsn;
use gpui::Global;
use libcdio_sys::{
    CdIo_t, cdio_cddap_speed_set, cdio_free, cdio_get_first_track_num, cdio_get_last_track_num,
    cdio_get_track_last_lsn, cdio_get_track_lsn, cdio_get_track_pregap_lsn, cdio_open_cd,
    cdio_set_speed,
};
use std::cell::RefCell;
use std::ffi::CString;
use std::sync::Arc;
use std::sync::RwLock;
use std::sync::Weak;

pub struct CdioManager {
    cdios: RefCell<Vec<Weak<CdioCd>>>,
}

impl CdioManager {
    pub fn new() -> Self {
        CdioManager {
            cdios: RefCell::new(Vec::new()),
        }
    }

    pub fn get_cd(&self, path: String) -> anyhow::Result<Arc<CdioCd>> {
        let mut cdios = self.cdios.borrow_mut();
        for cdio in cdios.iter() {
            if let Some(cdio) = cdio.upgrade() {
                if cdio.path == path {
                    return Ok(cdio.clone());
                }
            }
        }

        let cdio = Arc::new(CdioCd::new(path)?);
        cdios.push(Arc::downgrade(&cdio));
        Ok(cdio)
    }
}

impl Global for CdioManager {}

pub struct CdioCd {
    path: String,
    cdio_cd: *mut CdIo_t,
    paranoia: RwLock<Weak<CdioParanoia>>,
}

unsafe impl Send for CdioCd {}
unsafe impl Sync for CdioCd {}

#[derive(Debug)]
pub struct CdioCdTrack {
    pub track_number: u8,
    pub first_lsn: Lsn,
    pub last_lsn: Lsn,
}

impl CdioCd {
    pub fn new(path: String) -> anyhow::Result<Self> {
        let c_string = CString::new(path.clone().as_str())?;

        // SAFETY: todo?
        let cdio_cd = unsafe { cdio_open_cd(c_string.as_ptr()) };

        Ok(Self {
            path,
            cdio_cd,
            paranoia: Default::default(),
        })
    }

    pub fn first_track(&self) -> u8 {
        // SAFETY: The cdio_cd is owned by us and is guaranteed to be valid.
        unsafe { cdio_get_first_track_num(self.cdio_cd) }
    }

    pub fn last_track(&self) -> u8 {
        // SAFETY: The cdio_cd is owned by us and is guaranteed to be valid.
        unsafe { cdio_get_last_track_num(self.cdio_cd) }
    }

    pub fn track_information(&self, track: u8) -> Option<CdioCdTrack> {
        let first_track = self.first_track();
        let last_track = self.last_track();

        if track < first_track {
            // Return information about the pregap track.
            unsafe {
                Some(CdioCdTrack {
                    track_number: first_track,
                    first_lsn: Lsn(cdio_get_track_pregap_lsn(self.cdio_cd, first_track)),
                    last_lsn: Lsn(cdio_get_track_lsn(self.cdio_cd, first_track)),
                })
            }
        } else if track > last_track {
            None
        } else {
            unsafe {
                Some(CdioCdTrack {
                    track_number: track,
                    first_lsn: Lsn(cdio_get_track_lsn(self.cdio_cd, track)),
                    last_lsn: Lsn(cdio_get_track_last_lsn(self.cdio_cd, track)),
                })
            }
        }
    }

    /// Get the Cdio_t pointer
    ///
    /// # Safety
    /// This pointer is valid for the lifetime of the CdioCd and will be freed when it is dropped.
    pub unsafe fn as_ptr(&self) -> *mut CdIo_t {
        self.cdio_cd
    }

    pub fn paranoia(cdio_cd: Arc<CdioCd>) -> Arc<CdioParanoia> {
        let paranoia_borrow = cdio_cd.paranoia.read().unwrap();
        if let Some(paranoia) = paranoia_borrow.upgrade() {
            return paranoia;
        }
        drop(paranoia_borrow);

        let paranoia = Arc::new(CdioParanoia::new(cdio_cd.clone()).unwrap());
        let mut paranoia_borrow = cdio_cd.paranoia.write().unwrap();
        *paranoia_borrow = Arc::downgrade(&paranoia);
        paranoia
    }
}

impl Drop for CdioCd {
    fn drop(&mut self) {
        // SAFETY: The cdio_cd is owned by us and is guaranteed to be valid.
        unsafe { cdio_free(self.cdio_cd.cast()) }
    }
}
