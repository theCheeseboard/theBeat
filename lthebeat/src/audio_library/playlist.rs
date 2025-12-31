use sqlx::Arguments;
use crate::audio_library::database_query::{DatabaseQuery, DatabaseRecord};
use crate::audio_library::track::Track;
use sqlx::sqlite::{SqliteArguments, SqliteRow};
use sqlx::{Error, Row, SqlitePool};
use url::Url;

#[derive(Default)]
pub enum Playlist {
    Ok {
        id: usize,
        name: String,

        pool: SqlitePool,
    },

    #[default]
    Loading,
    Error,
}

impl DatabaseRecord for Playlist {
    fn read_from_row(&mut self, row: Result<SqliteRow, Error>, pool: &SqlitePool) {
        if let Ok(row) = row {
            let mut args = SqliteArguments::<'static>::default();

            *self = Playlist::Ok {
                id: row.get::<u32, _>("id") as usize,
                name: row.get::<String, _>("name"),
                pool: pool.clone(),
            }
        } else {
            *self = Playlist::Error;
        }
    }
}

impl Playlist {
    pub fn read_tracks(&self) -> impl Future<Output = anyhow::Result<DatabaseQuery<Track>>> {
        match self {
            Playlist::Ok { id, pool, .. } => {
                let mut args = SqliteArguments::<'static>::default();
                args.add(*id as u32).unwrap();
                DatabaseQuery::new(
                    Some(pool.clone()),
                    "SELECT
                         tracks.id as id,
                         tracks.url as url,
                         tracks.name as name,
                         album.name as album,
                         artist.name as artist,
                         tracks.track as track,
                         tracks.disc as disc
                     FROM playlist_tracks
                        JOIN tracks ON playlist_tracks.track_id = tracks.id
                            LEFT JOIN artist ON tracks.artist = artist.id
                            LEFT JOIN album ON tracks.album = album.id
                     WHERE playlist_id = ?"
                        .to_string(),
                    args,
                )
            }
            Playlist::Loading => {
                panic!("Playlist is still loading");
            }
            Playlist::Error => {
                panic!("Playlist is in an error state");
            }
        }
    }
}
