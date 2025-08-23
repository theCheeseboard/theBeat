use cntp_i18n::tr;
use gpui::{ImageSource, RenderImage, img};
use image::{Frame, ImageReader, Pixel, RgbaImage};
use smallvec::smallvec;
use std::cell::LazyCell;
use std::io::Cursor;
use std::path::Path;
use std::sync::{Arc, LazyLock, RwLock};
use std::time::Duration;
use url::Url;

#[derive(Default, Clone, Debug)]
pub struct AudioMetadata {
    pub url: Option<Url>,
    pub title: Option<String>,
    pub artist: Option<String>,
    pub album: Option<String>,
    pub duration: Option<Duration>,
    pub track_number: Option<u64>,
    pub total_track_number: Option<u64>,
    pub disc_number: Option<u64>,
    pub total_disc_number: Option<u64>,

    pub album_cover: Option<Arc<Art>>,
}

impl AudioMetadata {
    pub fn get_title(&self) -> String {
        self.title.clone().unwrap_or_else(|| {
            self.url
                .clone()
                .and_then(|url| {
                    let path = url.path();
                    let path = Path::new(path);
                    path.file_name()
                        .map(|file_name| file_name.to_str().unwrap().to_string())
                })
                .unwrap_or_else(|| tr!("TRACK_UNKNOWN_TITLE", "Track").into())
        })
    }
}

#[derive(Clone, Debug)]
pub struct Art {
    backing_store: Arc<Box<[u8]>>,
    rendered_image: Arc<RwLock<Option<Option<Arc<RenderImage>>>>>,
}

impl Art {
    pub fn new(backing_store: Box<[u8]>) -> Self {
        let backing_store = Arc::new(backing_store);
        Self {
            backing_store,
            rendered_image: Arc::new(RwLock::new(None)),
        }
    }

    pub fn render_image(&self) -> Option<Arc<RenderImage>> {
        let image = self.rendered_image.read().unwrap();
        if let Some(image) = image.clone() {
            image
        } else {
            drop(image);

            let mut image = self.rendered_image.write().unwrap();
            *image = Some(extract_image((*self.backing_store).clone()));

            image.clone().unwrap()
        }
    }
}

fn extract_image(backing_store: Box<[u8]>) -> Option<Arc<RenderImage>> {
    let mut image = ImageReader::new(Cursor::new(backing_store))
        .with_guessed_format()
        .ok()?
        .decode()
        .ok()?
        .into_rgba8();

    rgb_to_bgr(&mut image);

    let frame = Frame::new(image);
    Some(Arc::new(RenderImage::new(smallvec![frame])))
}

fn rgb_to_bgr(image: &mut RgbaImage) {
    image.pixels_mut().for_each(|v| {
        let slice = v.channels();
        *v = *image::Rgba::from_slice(&[slice[2], slice[1], slice[0], slice[3]]);
    });
}
