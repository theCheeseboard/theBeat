use crate::track_listing::playlist_selection_popover::PlaylistSelectionPopover;
use cntp_i18n::tr;
use contemporary::components::button::button;
use contemporary::components::context_menu::{ContextMenuExt, ContextMenuItem};
use contemporary::components::icon::icon;
use contemporary::components::skeleton::{SkeletonExt, skeleton, skeleton_row};
use contemporary::styling::theme::{Theme, ThemeStorage, VariableColor};
use gpui::prelude::FluentBuilder;
use gpui::{div, px, App, AppContext, AsyncApp, BorrowAppContext, ElementId, Entity, InteractiveElement, IntoElement, ParentElement, RenderOnce, StatefulInteractiveElement, Styled, Window};
use lthebeat::audio_library::database::Database;
use lthebeat::audio_library::track::Track;
use lthebeat::play_queue::PlayQueue;
use lthebeat::play_queue::media_item::MediaItem;
use std::rc::Rc;

#[derive(IntoElement)]
pub struct TrackElement {
    track: Entity<Track>,
    id: Option<u64>,
    context: Option<TrackContext>,
}

#[derive(Copy, Clone)]
pub enum TrackContext {
    InPlaylist { playlist_id: u64 },
}

pub fn track_element(track: Entity<Track>) -> TrackElement {
    TrackElement {
        track,
        id: None,
        context: None,
    }
}

impl TrackElement {
    pub fn custom_id(mut self, id: u64) -> TrackElement {
        self.id = Some(id);
        self
    }

    pub fn context(mut self, context: TrackContext) -> TrackElement {
        self.context = Some(context);
        self
    }
}

impl RenderOnce for TrackElement {
    fn render(self, window: &mut Window, cx: &mut App) -> impl IntoElement {
        match self.track.read(cx).clone() {
            Track::Ok {
                name,
                id,
                url,
                track,
                disc,
                artist,
                album,
                playlist_track_id,
            } => {
                let effective_id = self.id.unwrap_or(id as u64);
                let add_to_playlist_popover_open = window.use_keyed_state(
                    ElementId::NamedInteger("add_to_playlist_open".into(), effective_id),
                    cx,
                    |_, _| false,
                );
                let hovering = window.use_keyed_state(
                    ElementId::NamedInteger("hovering".into(), effective_id),
                    cx,
                    |_, _| false,
                );

                let theme = cx.theme();

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
                let mut context_menu = vec![
                    ContextMenuItem::separator().label(tr!("TRACK_CONTEXT_MENU_TITLE", "For {{track}}", track:quote=track_name)).build(),
                ];
                if let Some(TrackContext::InPlaylist { playlist_id }) = self.context {
                    let playlist_track_id = playlist_track_id.expect("TrackElement rendered with playlist context, but query does not return a playlist_track_id");
                    context_menu.push(
                        ContextMenuItem::menu_item()
                            .label(tr!("REMOVE_FROM_PLAYLIST", "Remove from playlist"))
                            .icon("list-remove")
                            .on_triggered({
                                move |_, _, cx| {
                                    let mutate = cx.global::<Database>().mutate().unwrap();
                                    cx.spawn(async move |cx: &mut AsyncApp| {
                                        mutate
                                            .remove_from_playlist(
                                                playlist_id as i64,
                                                playlist_track_id as i64,
                                                cx,
                                            )
                                            .await
                                            .unwrap();
                                    })
                                    .detach();
                                }
                            })
                            .build(),
                    );
                }
                context_menu.push(
                    ContextMenuItem::menu_item()
                        .label(tr!("ADD_TO_PLAYLIST", "Add to playlist..."))
                        .icon("list-add")
                        .on_triggered({
                            let add_to_playlist_popover_open = add_to_playlist_popover_open.clone();
                            move |_, _, cx| {
                                add_to_playlist_popover_open.write(cx, true);
                            }
                        })
                        .build(),
                );

                let url_clone = url.clone();
                div()
                    .id(ElementId::Integer(effective_id))
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
                                                    .on_click({
                                                        let add_to_playlist_popover_open =
                                                            add_to_playlist_popover_open.clone();
                                                        move |_, _, cx| {
                                                            add_to_playlist_popover_open
                                                                .write(cx, true);
                                                        }
                                                    }),
                                            ),
                                        ),
                                )
                            })
                            .child(div().id("hoverer").absolute().size_full().on_hover(
                                move |is_hovering, _, cx| {
                                    hovering_clone.write(cx, *is_hovering);
                                },
                            ))
                            .on_click(move |_, _, cx| {
                                let item = cx.new(|cx| MediaItem::new(url_clone.clone(), cx));
                                cx.update_global::<PlayQueue, ()>(|play_queue, cx| {
                                    play_queue.add_item(item, cx);
                                })
                            })
                            .with_context_menu(context_menu),
                    )
                    .child(PlaylistSelectionPopover {
                        visible: add_to_playlist_popover_open.clone(),
                        on_select: Rc::new(move |playlist, window, cx| {
                            let mutate = cx.global::<Database>().mutate().unwrap();
                            cx.spawn(async move |cx: &mut AsyncApp| {
                                mutate
                                    .add_to_playlist(playlist as i64, id as i64, cx)
                                    .await
                                    .unwrap();
                            })
                            .detach();
                        }),
                    })
                    .into_any_element()
            }
            Track::Loading => {
                let theme = cx.theme();
                div()
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
                    .into_any_element()
            }
            Track::Error => div()
                .child(tr!("LIBRARY_TRACKS_ERROR", "Error loading tracks"))
                .into_any_element(),
        }
    }
}
