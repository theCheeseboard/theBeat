use cntp_i18n::tr;
use std::path::Path;
use std::time::Duration;
use url::Url;

#[derive(Default, Clone, Debug)]
pub struct AudioMetadata {
    pub url: Option<Url>,
    pub title: Option<String>,
    pub artist: Option<String>,
    pub album: Option<String>,
    pub duration: Option<Duration>,
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
