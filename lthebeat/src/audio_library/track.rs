use crate::audio_library::database_query::DatabaseRecord;
use crate::play_queue::PlayQueue;
use crate::play_queue::media_item::MediaItem;
use cntp_i18n::tr;
use contemporary::components::spinner::spinner;
use gpui::{
    Context, Element, ElementId, InteractiveElement, IntoElement, ParentElement, Render,
    StatefulInteractiveElement, Window, div,
};
use sqlx::sqlite::SqliteRow;
use sqlx::{Error, Row};
use url::Url;

#[derive(Default)]
pub enum Track {
    Ok {
        id: usize,
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
            Track::Ok { name, id, url } => {
                let url_clone = url.clone();
                div()
                    .id(ElementId::from(*id))
                    .child(name.to_string())
                    .on_click(cx.listener(move |_, _, _, cx| {
                        let item = MediaItem::new(url_clone.clone(), cx);
                        let play_queue = cx.global_mut::<PlayQueue>();
                        play_queue.add_item(item);
                    }))
                    .into_any_element()
            }
            Track::Loading => div().child(spinner()).into_any_element(),
            Track::Error => div()
                .child(tr!("LIBRARY_TRACKS_ERROR", "Error loading tracks"))
                .into_any_element(),
        }
    }
}

impl DatabaseRecord for Track {
    fn read_from_row(&mut self, row: Result<SqliteRow, Error>) {
        if let Ok(row) = row {
            *self = Track::Ok {
                id: row.get::<u32, _>("id") as usize,
                url: Url::parse(row.get::<String, _>("url").as_str()).unwrap(),
                name: row.get::<String, _>("name"),
            }
        } else {
            *self = Track::Error;
        }
    }
}
