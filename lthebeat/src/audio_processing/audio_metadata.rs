use cntp_i18n::tr;
use gpui::{ImageSource, RenderImage, Rgba, img};
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
    pub track_number: Option<u32>,
    pub total_track_number: Option<u32>,
    pub disc_number: Option<u32>,
    pub total_disc_number: Option<u32>,

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
    pub backing_store: Arc<Box<[u8]>>,
    pub mime_type: String,
    rendered_image: Arc<RwLock<Option<Option<Arc<RenderImage>>>>>,
    average_color: Arc<RwLock<Option<Option<Rgba>>>>,
}

impl Art {
    pub fn new(backing_store: Box<[u8]>, mime_type: String) -> Self {
        let backing_store = Arc::new(backing_store);
        Self {
            backing_store,
            mime_type,
            rendered_image: Arc::new(RwLock::new(None)),
            average_color: Arc::new(RwLock::new(None)),
        }
    }

    pub fn render_image(&self) -> Option<Arc<RenderImage>> {
        let image = self.rendered_image.read().unwrap();
        if let Some(image) = image.clone() {
            image
        } else {
            drop(image);
            self.calculate_properties();
            self.render_image()
        }
    }

    pub fn average_color(&self) -> Option<Rgba> {
        let avg = self.average_color.read().unwrap();
        if let Some(avg) = *avg {
            avg
        } else {
            drop(avg);
            self.calculate_properties();
            self.average_color()
        }
    }

    fn calculate_properties(&self) {
        let mut image = self.rendered_image.write().unwrap();
        let mut avg = self.average_color.write().unwrap();
        if let Some((extracted_image, average_color)) = extract_image((*self.backing_store).clone())
        {
            *image = Some(Some(extracted_image));
            *avg = Some(Some(average_color));
        } else {
            *image = None;
            *avg = None;
        }
    }
}

fn extract_image(backing_store: Box<[u8]>) -> Option<(Arc<RenderImage>, Rgba)> {
    let mut image = ImageReader::new(Cursor::new(backing_store))
        .with_guessed_format()
        .ok()?
        .decode()
        .ok()?
        .into_rgba8();

    rgb_to_bgr(&mut image);

    let average_color = image.pixels().enumerate().fold(
        Rgba {
            r: 0.,
            g: 0.,
            b: 0.,
            a: 1.,
        },
        |acc, (i, rgba)| Rgba {
            r: acc.r + (rgba.channels()[0] as f32 / 255. - acc.r) / (i + 1) as f32,
            g: acc.g + (rgba.channels()[1] as f32 / 255. - acc.g) / (i + 1) as f32,
            b: acc.b + (rgba.channels()[2] as f32 / 255. - acc.b) / (i + 1) as f32,
            a: 1.,
        },
    );

    let frame = Frame::new(image);
    Some((Arc::new(RenderImage::new(smallvec![frame])), average_color))
}

fn rgb_to_bgr(image: &mut RgbaImage) {
    image.pixels_mut().for_each(|v| {
        let slice = v.channels();
        *v = *image::Rgba::from_slice(&[slice[2], slice[1], slice[0], slice[3]]);
    });
}
