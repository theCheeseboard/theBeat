use crate::cdio_paranoia_engine::cdio_paranoia::CdioParanoia;
use crate::cdio_paranoia_engine::lsn::Lsn;
use base64::Engine;
use base64::alphabet::{Alphabet, Symbol};
use base64::engine::GeneralPurpose;
use base64::engine::general_purpose::PAD;
use gpui::Global;
use libcdio_sys::{
    CdIo_t, cdio_free, cdio_get_cdtext, cdio_get_disc_last_lsn, cdio_get_first_track_num,
    cdio_get_last_track_num, cdio_get_track_last_lsn, cdio_get_track_lsn,
    cdio_get_track_pregap_lsn, cdio_open_cd, cdtext_field_t, cdtext_field_t_CDTEXT_FIELD_COMPOSER,
    cdtext_field_t_CDTEXT_FIELD_GENRE, cdtext_field_t_CDTEXT_FIELD_MESSAGE,
    cdtext_field_t_CDTEXT_FIELD_PERFORMER, cdtext_field_t_CDTEXT_FIELD_SONGWRITER,
    cdtext_field_t_CDTEXT_FIELD_TITLE, cdtext_get_const, cdtext_t,
};
use sha1::{Digest, Sha1};
use std::cell::RefCell;
use std::ffi::{CStr, CString};
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

    pub fn get_cd(&self, path: &str) -> anyhow::Result<Arc<CdioCd>> {
        let mut cdios = self.cdios.borrow_mut();
        for cdio in cdios.iter() {
            if let Some(cdio) = cdio.upgrade() {
                if cdio.path == path {
                    return Ok(cdio.clone());
                }
            }
        }

        let cdio = Arc::new(CdioCd::new(path.to_string())?);
        cdios.push(Arc::downgrade(&cdio));
        Ok(cdio)
    }
}

impl Global for CdioManager {}

pub struct CdioCd {
    path: String,
    cdio_cd: *mut CdIo_t,
    cdio_cd_text: Option<*mut cdtext_t>,
    paranoia: RwLock<Weak<CdioParanoia>>,
}

unsafe impl Send for CdioCd {}
unsafe impl Sync for CdioCd {}

#[derive(Debug)]
pub struct CdioCdTrack {
    pub track_number: u8,
    pub first_lsn: Lsn,
    pub last_lsn: Lsn,
    pub cd_text: Option<CdText>,
}

#[derive(Debug)]
pub struct CdText {
    pub title: Option<String>,
    pub performer: Option<String>,
    pub songwriter: Option<String>,
    pub composer: Option<String>,
    pub message: Option<String>,
    pub genre: Option<String>,
}

impl CdText {
    fn read_from_cdtext_t(cdtext: *const cdtext_t, track: u8) -> CdText {
        CdText {
            title: Self::read_cd_text(cdtext, cdtext_field_t_CDTEXT_FIELD_TITLE, track),
            performer: Self::read_cd_text(cdtext, cdtext_field_t_CDTEXT_FIELD_PERFORMER, track),
            songwriter: Self::read_cd_text(cdtext, cdtext_field_t_CDTEXT_FIELD_SONGWRITER, track),
            composer: Self::read_cd_text(cdtext, cdtext_field_t_CDTEXT_FIELD_COMPOSER, track),
            message: Self::read_cd_text(cdtext, cdtext_field_t_CDTEXT_FIELD_MESSAGE, track),
            genre: Self::read_cd_text(cdtext, cdtext_field_t_CDTEXT_FIELD_GENRE, track),
        }
    }

    fn read_cd_text(cdtext: *const cdtext_t, field: cdtext_field_t, track: u8) -> Option<String> {
        unsafe {
            let text = cdtext_get_const(cdtext, field, track);
            if text.is_null() {
                None
            } else {
                Some(CStr::from_ptr(text).to_string_lossy().to_string())
            }
        }
    }
}

impl CdioCd {
    pub fn new(path: String) -> anyhow::Result<Self> {
        let c_string = CString::new(path.clone().as_str())?;

        // SAFETY: todo?
        let cdio_cd = unsafe { cdio_open_cd(c_string.as_ptr()) };
        let cdio_cd_text = unsafe { cdio_get_cdtext(cdio_cd) };

        Ok(Self {
            path,
            cdio_cd,
            cdio_cd_text: if cdio_cd_text.is_null() {
                None
            } else {
                Some(cdio_cd_text)
            },
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

        let cd_text = self
            .cdio_cd_text
            .map(|cdio_cd_text| CdText::read_from_cdtext_t(cdio_cd_text, track));

        if track < first_track {
            // Return information about the pregap track.
            unsafe {
                Some(CdioCdTrack {
                    track_number: first_track,
                    first_lsn: Lsn(cdio_get_track_pregap_lsn(self.cdio_cd, first_track)),
                    last_lsn: Lsn(cdio_get_track_lsn(self.cdio_cd, first_track)),
                    cd_text,
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
                    cd_text,
                })
            }
        }
    }

    pub fn lead_out_offset(&self) -> Lsn {
        unsafe { Lsn(cdio_get_disc_last_lsn(self.cdio_cd)) }
    }

    pub fn disc_cd_text(&self) -> Option<CdText> {
        self.cdio_cd_text
            .map(|cdio_cd_text| CdText::read_from_cdtext_t(cdio_cd_text, 0))
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

    pub fn musicbrainz_disc_id(&self) -> String {
        let mut hasher = Sha1::new();

        // First track number
        hasher.update(format!("{:02X}", self.first_track()));
        // Last track number
        hasher.update(format!("{:02X}", self.last_track()));
        // Lead out offset
        hasher.update(format!("{:08X}", self.lead_out_offset().0 + 150));
        for i in 1..=99 {
            if let Some(track_information) = self.track_information(i) {
                hasher.update(format!("{:08X}", track_information.first_lsn.0 + 150));
            } else {
                hasher.update(format!("{:08X}", 0));
            }
        }

        let hash = hasher.finalize();
        let musicbrainz_base64_encoder = GeneralPurpose::new(
            &Alphabet::new_with_padding(
                "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789._",
                Symbol::new(b'-').unwrap(),
            )
            .unwrap(),
            PAD,
        );
        musicbrainz_base64_encoder.encode(hash.as_slice())
    }
}

impl Drop for CdioCd {
    fn drop(&mut self) {
        if let Some(cd_text) = self.cdio_cd_text.take() {
            // SAFETY: The cdtext_t s owned by us
            unsafe {
                cdio_free(cd_text.cast());
            }
        }

        // SAFETY: The cdio_cd is owned by us and is guaranteed to be valid.
        unsafe { cdio_free(self.cdio_cd.cast()) }
    }
}

impl CdioCdTrack {
    pub fn max_len(&self) -> Lsn {
        self.last_lsn - self.first_lsn
    }
}