use lthebeat::audio_processing::audio_metadata::AudioMetadata;

pub trait TrackMetadata {
    fn supplementary_text(&self) -> String;
}

impl TrackMetadata for AudioMetadata {
    fn supplementary_text(&self) -> String {
        let mut s = Vec::new();
        if let Some(artist) = &self.artist {
            s.push(artist.clone());
        }
        if let Some(album) = &self.album {
            s.push(album.clone());
        }
        s.join(" • ")
    }
}
