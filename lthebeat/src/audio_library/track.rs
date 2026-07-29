use crate::audio_library::database::Database;
use crate::audio_library::database_query::{DatabaseQuery, DatabaseRecord};
use crate::audio_library::playlist::Playlist;
use crate::play_queue::PlayQueue;
use crate::play_queue::media_item::MediaItem;
use cntp_i18n::{Quote, tr};
use contemporary::components::button::button;
use contemporary::components::constrainer::constrainer;
use contemporary::components::context_menu::{
    ContextMenuActionEvent, ContextMenuExt, ContextMenuItem,
};
use contemporary::components::grandstand::grandstand;
use contemporary::components::icon::icon;
use contemporary::components::layer::layer;
use contemporary::components::popover::popover;
use contemporary::components::skeleton::{SkeletonExt, skeleton, skeleton_row};
use contemporary::components::spinner::spinner;
use contemporary::styling::theme::{Theme, VariableColor};
use gpui::prelude::FluentBuilder;
use gpui::{
    App, AsyncApp, BorrowAppContext, ClickEvent, Context, Element, ElementId, Entity,
    InteractiveElement, IntoElement, ListSizingBehavior, ParentElement, Render, RenderOnce,
    StatefulInteractiveElement, Styled, WeakEntity, Window, div, px, rgb, uniform_list,
};
use sqlx::sqlite::SqliteRow;
use sqlx::{Error, Row, SqlitePool};
use std::cell::RefCell;
use std::rc::Rc;
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
        let add_to_playlist_popover_open = window.use_state(cx, |_, _| false);
        let hovering = window.use_state(cx, |_, _| false);

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
                let hovering_clone = hovering.clone();

                let mut supps = Vec::new();
                if let Some(artist) = artist {
                    supps.push(tr!("TRACK_ARTIST", "By {{artist}}", artist = artist).to_string());
                }
                if let Some(album) = album {
                    supps.push(tr!("TRACK_ALBUM", "On {{album}}", album = album).to_string());
                }

                let track_name = name
                    .clone()
                    .unwrap_or_else(|| {
                        url.to_file_path()
                            .map(|path| path.file_name().unwrap().to_str().unwrap().to_string())
                            .unwrap_or_else(|_| url.to_string())
                    })
                    .to_string();
                let context_menu = vec![
                    ContextMenuItem::separator().label(tr!("TRACK_CONTEXT_MENU_TITLE", "For {{track}}", track:Quote=track_name)).build(),
                    ContextMenuItem::menu_item().label(tr!("ADD_TO_PLAYLIST", "Add to playlist...")).icon("list-add").on_triggered({let add_to_playlist_popover_open = add_to_playlist_popover_open.clone(); move |_, _, cx| {
                        add_to_playlist_popover_open.write(cx, true);
                    }}).build()
                ];

                let url_clone = url.clone();
                div()
                    .id(ElementId::from(*id))
                    .child(
                        div()
                            .id("clickable")
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
                                    .child(div().child(track_name))
                                    .child(
                                        div()
                                            .child(supps.join(" • "))
                                            .text_color(theme.foreground.disabled()),
                                    ),
                            )
                            .when(*hovering.read(cx), |david| {
                                david.child(
                                    div()
                                        .absolute()
                                        .size_full()
                                        .flex()
                                        .p(px(8.))
                                        .child(div().flex_grow(1.))
                                        .child(
                                            div().flex().block_mouse_except_scroll().child(
                                                button("add-to-playlist-button")
                                                    .child(icon("view-media-playlist"))
                                                    .on_click(cx.listener({
                                                        let add_to_playlist_popover_open =
                                                            add_to_playlist_popover_open.clone();
                                                        move |this, _, _, cx| {
                                                            add_to_playlist_popover_open
                                                                .write(cx, true);
                                                        }
                                                    })),
                                            ),
                                        ),
                                )
                            })
                            .child(div().id("hoverer").absolute().size_full().on_hover(
                                move |is_hovering, _, cx| {
                                    hovering_clone.write(cx, *is_hovering);
                                },
                            ))
                            .on_click(cx.listener(move |_, _, _, cx| {
                                let item = MediaItem::new(url_clone.clone(), cx);
                                cx.update_global::<PlayQueue, ()>(|play_queue, cx| {
                                    play_queue.add_item(item, cx);
                                })
                            }))
                            .with_context_menu(context_menu),
                    )
                    .child(PlaylistSelectionPopover {
                        visible: add_to_playlist_popover_open.clone(),
                    })
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

#[derive(IntoElement)]
struct PlaylistSelectionPopover {
    visible: Entity<bool>,
}

impl RenderOnce for PlaylistSelectionPopover {
    fn render(self, window: &mut Window, cx: &mut App) -> impl IntoElement {
        let popover_open_clone = self.visible.clone();

        let playlists_query =
            window.use_state::<Option<RefCell<DatabaseQuery<Playlist>>>>(cx, |_, cx| {
                let database = cx.global::<Database>();
                let query = database.query_all_playlists();

                cx.spawn(
                    async move |playlists_view: WeakEntity<
                        Option<RefCell<DatabaseQuery<Playlist>>>,
                    >,
                                cx: &mut AsyncApp| {
                        let playlist_query = query.await;
                        if let Some(playlists_view) = playlists_view.upgrade() {
                            playlists_view.write(
                                cx,
                                playlist_query
                                    .ok()
                                    .map(|playlist_query| RefCell::new(playlist_query)),
                            )
                        }
                    },
                )
                .detach();

                None
            });

        popover("add-to-playlist-popover")
            .visible(*self.visible.read(cx))
            .size_neg(100.)
            .anchor_bottom()
            .content(
                div()
                    .flex()
                    .flex_col()
                    .gap(px(9.))
                    .child(
                        grandstand("add-to-playlist-grandstand")
                            .text(tr!("ADD_TO_PLAYLIST_TITLE", "Add to playlist"))
                            .on_back_click(move |_, _, cx| {
                                popover_open_clone.write(cx, false);
                            }),
                    )
                    .child(
                        constrainer("add-to-playlist-constrainer").child(
                            layer()
                                .flex()
                                .flex_col()
                                .p(px(8.))
                                .w_full()
                                .child(tr!("ADD_TO_PLAYLIST_PROMPT", "Which playlist?"))
                                .when_some(playlists_query.read(cx).as_ref(), |david, query| {
                                    let playlists_query = playlists_query.clone();
                                    david.child(
                                        uniform_list(
                                            "playlist_list",
                                            query.borrow().count(),
                                            move |range, _, cx| {
                                                playlists_query.update(cx, |playlists_query, cx| {
                                                    let playlists_query =
                                                        playlists_query.as_ref().unwrap();
                                                    range
                                                        .map(|index| {
                                                            match playlists_query
                                                                .borrow_mut()
                                                                .get(index, cx)
                                                                .read(cx)
                                                            {
                                                                Playlist::Ok {
                                                                    name, id, ..
                                                                } => div().child(name.clone()),
                                                                _ => div(),
                                                            }
                                                        })
                                                        .collect()
                                                })
                                            },
                                        )
                                        .with_sizing_behavior(ListSizingBehavior::Infer),
                                    )
                                }),
                        ),
                    )
                    .into_any_element(),
            )
    }
}
