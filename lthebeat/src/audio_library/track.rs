use crate::audio_library::database_query::DatabaseRecord;
use sqlx::sqlite::SqliteRow;
use sqlx::{Error, Row, SqlitePool};
use url::Url;

#[derive(Default, Clone)]
pub enum Track {
    Ok {
        id: usize,
        track: Option<u32>,
        disc: Option<u32>,
        url: Url,
        name: Option<String>,
        artist: Option<String>,
        album: Option<String>,

        playlist_track_id: Option<u32>
    },

    #[default]
    Loading,
    Error,
}

impl DatabaseRecord for Track {
    fn read_from_row(&mut self, row: Result<SqliteRow, Error>, _: &SqlitePool) {
        if let Ok(row) = row {
            *self = Track::Ok {
                id: row.get::<u32, _>("id") as usize,
                track: row.get::<Option<u32>, _>("track"),
                disc: row.get::<Option<u32>, _>("disc"),
                url: Url::parse(row.get::<String, _>("url").as_str()).unwrap(),
                name: row.get::<Option<String>, _>("name"),
                artist: row.get::<Option<String>, _>("artist"),
                album: row.get::<Option<String>, _>("album"),

                playlist_track_id: row.try_get::<Option<u32>, _>("playlist_track_id").ok().flatten(),
            }
        } else {
            *self = Track::Error;
        }
    }
}
