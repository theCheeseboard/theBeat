use crate::track_metadata::TrackMetadata;
use cntp_i18n::tr;
use contemporary::components::context_menu::{ContextMenuExt, ContextMenuItem};
use contemporary::components::grandstand::grandstand;
use contemporary::components::icon::icon;
use contemporary::components::interstitial::interstitial;
use contemporary::components::layer::layer;
use contemporary::components::pager::fade_animation::FadeAnimation;
use contemporary::components::pager::pager;
use contemporary::styling::theme::{Theme, VariableColor};
use gpui::http_client::Url;
use gpui::prelude::FluentBuilder;
use gpui::{div, img, list, px, rgba, App, AppContext, BorrowAppContext, ElementId, ExternalPaths, ImageSource, InteractiveElement, IntoElement, ListAlignment, ListState, ParentElement, Refineable, RenderOnce, StatefulInteractiveElement, StyleRefinement, Styled, Window};
use lthebeat::audio_processing::audio_controller::AudioController;
use lthebeat::play_queue::DisplayQueueItem;
use lthebeat::play_queue::media_item::MediaItem;

#[derive(IntoElement)]
pub struct PlayQueue {
    style: StyleRefinement,
}

pub fn play_queue() -> PlayQueue {
    PlayQueue {
        style: Default::default(),
    }
}

impl RenderOnce for PlayQueue {
    fn render(self, window: &mut Window, cx: &mut App) -> impl IntoElement {
        let list_state =
            window.use_state(cx, |_, _| ListState::new(0, ListAlignment::Top, px(200.)));

        let audio_controller = cx.global::<AudioController>();
        let play_queue = cx.global::<lthebeat::play_queue::PlayQueue>();
        let list_state = list_state.read(cx);
        let display_queue = play_queue.display_queue(cx);
        if display_queue.len() != list_state.item_count() {
            list_state.reset(display_queue.len());
        }
        let current_track = audio_controller
            .current_track()
            .map(|current_track| current_track.entity_id());

        let mut div = layer()
            .w(px(300.))
            .flex()
            .flex_col()
            .child(
                grandstand("queue-grandstand")
                    .text(tr!("QUEUE_TITLE", "Queue"))
                    .pt(px(36.)),
            )
            .child(
                div()
                    .id("queue")
                    .overflow_y_scroll()
                    .flex_grow(1.)
                    .flex()
                    .flex_col()
                    .child(
                        pager("queue-pager", if display_queue.is_empty() { 0 } else { 1 }).flex_grow(1.)
                            .animation(FadeAnimation::new())
                            .page(interstitial()
                                .title(tr!("QUEUE_EMPTY_TITLE", "Nothing here!"))
                                .message(tr!("QUEUE_EMPTY_MESSAGE", "Select a track or drop something here!"))
                                .h_full()
                                .into_any_element())
                            .page(div().h_full()
                                .with_context_menu([
                                    ContextMenuItem::separator()
                                        .label(tr!("QUEUE_CONTEXT_MENU_TITLE", "For Queue"))
                                        .build(),
                                    ContextMenuItem::menu_item()
                                        .label(tr!("QUEUE_CONTEXT_MENU_CLEAR", "Clear Queue"))
                                        .icon("edit-delete")
                                        .on_triggered(|_, _, cx| {
                                            let play_queue = cx.global_mut::<lthebeat::play_queue::PlayQueue>();
                                            play_queue.clear();
                                        })
                                        .build(),
                                ]).child(
                                list(list_state.clone(), move |i, _, cx| {
                                    let theme = cx.global::<Theme>();

                                    let queue_context_menu_items = [
                                        ContextMenuItem::separator()
                                            .label(tr!("QUEUE_CONTEXT_MENU_TITLE"))
                                            .build(),
                                        ContextMenuItem::menu_item()
                                            .label(tr!("QUEUE_CONTEXT_MENU_CLEAR"))
                                            .icon("edit-delete")
                                            .on_triggered(|_, _, cx| {
                                                let play_queue =
                                                    cx.global_mut::<lthebeat::play_queue::PlayQueue>();
                                                play_queue.clear();
                                            })
                                            .build(),
                                    ];

                                    match display_queue.get(i).unwrap().clone() {
                                        DisplayQueueItem::SingleItemGroup(item_entity) => {
                                            let item_entity_2 = item_entity.clone();
                                            let item = item_entity.read(cx);
                                            let item_title = item.meta().get_title();

                                            let cover = item
                                                .meta()
                                                .clone()
                                                .album_cover
                                                .and_then(|album_cover| album_cover.render_image())
                                                .clone();

                                            div()
                                                .id(ElementId::from(i))
                                                .flex()
                                                .gap(px(3.))
                                                .w_full()
                                                .child(
                                                    div()
                                                        .size(px(48.))
                                                        .when_some(cover, |div, album_cover| {
                                                            div.child(
                                                                img(ImageSource::Render(album_cover))
                                                                    .h_full()
                                                                    .w_full(),
                                                            )
                                                        })
                                                        .when(
                                                            current_track == Some(item_entity.entity_id()),
                                                            |david| {
                                                                david.child(
                                                                    div()
                                                                        .absolute()
                                                                        .left_0()
                                                                        .top_0()
                                                                        .size_full()
                                                                        .flex()
                                                                        .items_center()
                                                                        .justify_center()
                                                                        .bg(rgba(0x00000070))
                                                                        .child(icon(
                                                                            "media-playback-start",
                                                                        )),
                                                                )
                                                            },
                                                        ),
                                                )
                                                .child(
                                                    div()
                                                        .flex()
                                                        .flex_col()
                                                        .flex_grow(1.)
                                                        .overflow_hidden()
                                                        .gap(px(3.))
                                                        .child(div().overflow_hidden().text_ellipsis().child(item_title.clone()))
                                                        .child(
                                                            div().overflow_hidden().text_ellipsis()
                                                                .text_color(theme.foreground.disabled())
                                                                .child(item.meta().supplementary_text()),
                                                        ),
                                                )
                                                .on_click(move |_, _, cx| {
                                                    // Jump to this track
                                                    let play_queue =
                                                        cx.global_mut::<lthebeat::play_queue::PlayQueue>();
                                                    play_queue.skip_to_item(item_entity.clone());
                                                })
                                                .with_context_menu([
                                                    ContextMenuItem::separator()
                                                        .label(tr!("QUEUE_ITEM_CONTEXT_MENU_TITLE", "For {{track}}", track:quote=item_title))
                                                        .build(),
                                                    ContextMenuItem::menu_item()
                                                        .label(tr!("QUEUE_ITEM_CONTEXT_MENU_REMOVE", "Remove from Queue"))
                                                        .icon("edit-delete")
                                                        .on_triggered(move |_, _, cx| {
                                                            let play_queue = cx.global_mut::<lthebeat::play_queue::PlayQueue>();
                                                            play_queue.remove_item(item_entity_2.clone());
                                                        })
                                                        .build(),
                                                ].into_iter().chain(queue_context_menu_items))
                                                .into_any_element()
                                        }
                                        DisplayQueueItem::GroupItem(item_entity) => {
                                            let item_entity_2 = item_entity.clone();
                                            let item = item_entity.read(cx);
                                            let item_title = item.meta().get_title();

                                            div()
                                                .id(ElementId::from(i))
                                                .flex()
                                                .w_full()
                                                .gap(px(3.))
                                                .child(
                                                    div()
                                                        .size(px(20.))
                                                        .flex()
                                                        .items_center()
                                                        .justify_center()
                                                        .child(div().when_else(
                                                            current_track == Some(item_entity.entity_id()),
                                                            |div| {
                                                                div.child(icon(
                                                                    "media-playback-start",
                                                                ))
                                                            },
                                                            |div| {
                                                                div.text_color(theme.foreground.disabled())
                                                                    .child(
                                                                        item.meta()
                                                                            .track_number
                                                                            .map(|track_number| {
                                                                                track_number.to_string()
                                                                            })
                                                                            .unwrap_or("-".to_string()),
                                                                    )
                                                            },
                                                        )),
                                                )
                                                .child(div().overflow_hidden().text_ellipsis().child(item.meta().get_title()))
                                                .on_click(move |_, _, cx| {
                                                    // Jump to this track
                                                    let play_queue =
                                                        cx.global_mut::<lthebeat::play_queue::PlayQueue>();
                                                    play_queue.skip_to_item(item_entity.clone());
                                                })
                                                .with_context_menu([
                                                    ContextMenuItem::separator()
                                                        .label(tr!("QUEUE_ITEM_CONTEXT_MENU_TITLE", track:quote=item_title))
                                                        .build(),
                                                    ContextMenuItem::menu_item()
                                                        .label(tr!("QUEUE_ITEM_CONTEXT_MENU_REMOVE"))
                                                        .icon("edit-delete")
                                                        .on_triggered(move |_, _, cx| {
                                                            let play_queue = cx.global_mut::<lthebeat::play_queue::PlayQueue>();
                                                            play_queue.remove_item(item_entity_2.clone());
                                                        })
                                                        .build(),
                                                ].into_iter().chain(queue_context_menu_items))
                                                .into_any_element()
                                        }
                                        DisplayQueueItem::GroupHeader(item_entity) => {
                                            let item = item_entity.read(cx);

                                            let cover = item
                                                .meta()
                                                .clone()
                                                .album_cover
                                                .and_then(|album_cover| album_cover.render_image())
                                                .clone();

                                            div()
                                                .id(ElementId::from(i))
                                                .flex()
                                                .w_full()
                                                .gap(px(3.))
                                                .child(div().size(px(48.)).when_some(
                                                    cover,
                                                    |div, album_cover| {
                                                        div.child(
                                                            img(ImageSource::Render(album_cover))
                                                                .h_full()
                                                                .w_full(),
                                                        )
                                                    },
                                                ))
                                                .child(
                                                    div()
                                                        .flex()
                                                        .flex_col()
                                                        .flex_grow(1.)
                                                        .overflow_hidden()
                                                        .gap(px(3.))
                                                        .child(div()
                                                            .overflow_hidden()
                                                            .text_ellipsis()
                                                            .child(item.meta().album.clone().unwrap_or(
                                                                tr!("UNKNOWN_ALBUM", "Unknown Album").into(),
                                                            )))
                                                        .child(
                                                            div()
                                                                .overflow_hidden()
                                                                .text_ellipsis()
                                                                .text_color(theme.foreground.disabled())
                                                                .child(
                                                                    item.meta().artist.clone().unwrap_or(
                                                                        tr!(
                                                                    "UNKNOWN_ARTIST",
                                                                    "Unknown Artist"
                                                                ).into(),
                                                                    ),
                                                                ),
                                                        ),
                                                )
                                                .on_click(move |_, _, cx| {
                                                    // Jump to this track
                                                    let play_queue =
                                                        cx.global_mut::<lthebeat::play_queue::PlayQueue>();
                                                    play_queue.skip_to_item(item_entity.clone());
                                                })
                                                .with_context_menu(queue_context_menu_items)
                                                .into_any_element()
                                        }
                                    }
                                }).flex().flex_col().h_full()).into_any_element()),
                    ).child(
                    div()
                        .absolute()
                        .left_0()
                        .top_0()
                        .size_full()
                        .on_drop(|event: &ExternalPaths, _, cx| {
                            for path in event.paths() {
                                let url = Url::from_file_path(path).unwrap();
                                let item = cx.new(|cx| MediaItem::new(url, cx));
                                cx.update_global::<lthebeat::play_queue::PlayQueue, ()>(|play_queue, cx| {
                                    play_queue.add_item(item, cx);
                                })
                            }
                        })),
            );
        div.style().refine(&self.style);

        div
    }
}

impl Styled for PlayQueue {
    fn style(&mut self) -> &mut StyleRefinement {
        &mut self.style
    }
}
