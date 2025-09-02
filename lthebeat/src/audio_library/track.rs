use crate::audio_library::database_query::DatabaseRecord;
use cntp_i18n::tr;
use contemporary::components::spinner::spinner;
use gpui::{Context, Element, IntoElement, ParentElement, Render, Window, div};
use sqlx::sqlite::SqliteRow;
use sqlx::{Error, Row};
use url::Url;

#[derive(Default)]
pub enum Track {
    Ok {
        url: Url,
        name: String,
    },

    #[default]
    Loading,
    Error,
}

impl Render for Track {
    fn render(&mut self, window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        match self {
            Track::Ok { name, .. } => div().child(name.to_string()),
            Track::Loading => div().child(spinner()),
            Track::Error => div().child(tr!("LIBRARY_TRACKS_ERROR", "Error loading tracks")),
        }
    }
}

impl DatabaseRecord for Track {
    fn read_from_row(&mut self, row: Result<SqliteRow, Error>) {
        if let Ok(row) = row {
            *self = Track::Ok {
                url: Url::parse(row.get::<String, _>("url").as_str()).unwrap(),
                name: row.get::<String, _>("name"),
            }
        } else {
            *self = Track::Error;
        }
    }
}
