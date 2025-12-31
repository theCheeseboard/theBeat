use crate::audio_library::database_query::DatabaseRecord;
use crate::play_queue::PlayQueue;
use crate::play_queue::media_item::MediaItem;
use cntp_i18n::{tr, Quote};
use contemporary::components::context_menu::{ContextMenuExt, ContextMenuItem};
use contemporary::components::skeleton::{SkeletonExt, skeleton, skeleton_row};
use contemporary::components::spinner::spinner;
use contemporary::styling::theme::{Theme, VariableColor};
use gpui::{BorrowAppContext, Context, Element, ElementId, InteractiveElement, IntoElement, ParentElement, Render, StatefulInteractiveElement, Styled, Window, div, px};
use sqlx::sqlite::SqliteRow;
use sqlx::{Error, Row, SqlitePool};
use url::Url;

#[derive(Default)]
pub enum Track {
    Ok {
        id: usize,
        track: Option<u32>,
        disc: Option<u32>,
        url: Url,
        name: Option<String>,
        artist: Option<String>,
        album: Option<String>,
    },

    #[default]
    Loading,
    Error,
}

impl Render for Track {
    fn render(&mut self, window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let theme = cx.global::<Theme>();
        match self {
            Track::Ok {
                name,
                id,
                url,
                track,
                disc,
                artist,
                album,
            } => {
                let mut supps = Vec::new();
                if let Some(artist) = artist {
                    supps.push(tr!("TRACK_ARTIST", "By {{artist}}", artist = artist).to_string());
                }
                if let Some(album) = album {
                    supps.push(tr!("TRACK_ALBUM", "On {{album}}", album = album).to_string());
                }
                
                let track_name = name.clone()
                    .unwrap_or_else(|| {
                        url.to_file_path()
                            .map(|path| {
                                path.file_name()
                                    .unwrap()
                                    .to_str()
                                    .unwrap()
                                    .to_string()
                            })
                            .unwrap_or_else(|_| url.to_string())
                    })
                    .to_string();
                let context_menu = vec![
                    ContextMenuItem::separator().label(tr!("TRACK_CONTEXT_MENU_TITLE", "For {{track}}", track:Quote=track_name)).build(),
                    ContextMenuItem::menu_item().label(tr!("ADD_TO_PLAYLIST", "Add to playlist...")).icon("list-add").build()
                ];

                let url_clone = url.clone();
                div()
                    .id(ElementId::from(*id))
                    .flex()
                    .items_center()
                    .child(
                        div()
                            .child(
                                track
                                    .map(|track| track.to_string())
                                    .unwrap_or("-".to_string()),
                            )
                            .text_center()
                            .h(theme.system_font_size * 2 + px(12.))
                            .w(theme.system_font_size * 3 + px(12.))
                            .p(px(4.))
                            .text_color(theme.foreground.disabled())
                            .text_size(theme.system_font_size * 2),
                    )
                    .child(
                        div()
                            .flex()
                            .flex_col()
                            .child(
                                div().child(
                                    track_name,
                                ),
                            )
                            .child(
                                div()
                                    .child(supps.join(" • "))
                                    .text_color(theme.foreground.disabled()),
                            ),
                    )
                    .on_click(cx.listener(move |_, _, _, cx| {
                        let item = MediaItem::new(url_clone.clone(), cx);
                        cx.update_global::<PlayQueue, ()>(|play_queue, cx| {
                            play_queue.add_item(item, cx);
                        })
                    }))
                    .with_context_menu(context_menu)
                    .into_any_element()
            }
            Track::Loading => div()
                .flex()
                .child(
                    div()
                        .child("-")
                        .size(theme.system_font_size * 2 + px(12.))
                        .p(px(4.))
                        .into_skeleton("skel-track-no"),
                )
                .child(
                    div()
                        .flex()
                        .flex_col()
                        .child(
                            skeleton_row("skel-title")
                                .chunk("This is the track name")
                                .chunk("00:00"),
                        )
                        .child(skeleton("skel-supps").child("Supplementary Data")),
                )
                .into_any_element(),
            Track::Error => div()
                .child(tr!("LIBRARY_TRACKS_ERROR", "Error loading tracks"))
                .into_any_element(),
        }
    }
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
            }
        } else {
            *self = Track::Error;
        }
    }
}
