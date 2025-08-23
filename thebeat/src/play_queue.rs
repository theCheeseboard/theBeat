use crate::track_metadata::TrackMetadata;
use cntp_i18n::tr;
use contemporary::components::grandstand::grandstand;
use contemporary::components::icon::icon;
use contemporary::components::layer::layer;
use contemporary::styling::theme::{Theme, VariableColor};
use gpui::ListSizingBehavior::Infer;
use gpui::prelude::FluentBuilder;
use gpui::{
    App, ElementId, InteractiveElement, IntoElement, ListAlignment, ListState, ParentElement,
    Refineable, RenderOnce, StatefulInteractiveElement, StyleRefinement, Styled, Window, div, list,
    px, rgb, rgba,
};
use lthebeat::audio_processing::audio_controller::AudioController;
use lthebeat::play_queue::DisplayQueueItem;

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
        let list_state = window.use_state(cx, |_, _| ListState::new(0, ListAlignment::Top, px(0.)));

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
                div().id("queue").overflow_y_scroll().flex_grow().child(
                    list(list_state.clone(), move |i, _, cx| {
                        let theme = cx.global::<Theme>();
                        let play_queue = cx.global::<lthebeat::play_queue::PlayQueue>();
                        match display_queue.get(i).unwrap().clone() {
                            DisplayQueueItem::SingleItemGroup(item_entity) => {
                                let item = item_entity.read(cx);

                                div()
                                    .id(ElementId::from(i))
                                    .flex()
                                    .gap(px(3.))
                                    .child(div().size(px(48.)).bg(rgb(0xFF0000)).when(
                                        current_track == Some(item_entity.entity_id()),
                                        |david| {
                                            david.child(
                                                div()
                                                    .size_full()
                                                    .flex()
                                                    .items_center()
                                                    .justify_center()
                                                    .bg(rgba(0x00000070))
                                                    .child(icon("media-playback-start".into())),
                                            )
                                        },
                                    ))
                                    .child(
                                        div()
                                            .flex()
                                            .flex_col()
                                            .gap(px(3.))
                                            .child(item.meta.get_title())
                                            .child(
                                                div()
                                                    .text_color(theme.foreground.disabled())
                                                    .child(item.meta.supplementary_text()),
                                            ),
                                    )
                                    .on_click(move |_, _, cx| {
                                        // Jump to this track
                                        let play_queue =
                                            cx.global_mut::<lthebeat::play_queue::PlayQueue>();
                                        play_queue.skip_to_item(item_entity.clone());
                                    })
                                    .into_any_element()
                            }
                            DisplayQueueItem::GroupItem(item_entity) => {
                                let item = item_entity.read(cx);

                                div()
                                    .id(ElementId::from(i))
                                    .flex()
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
                                                    div.child(icon("media-playback-start".into()))
                                                },
                                                |div| {
                                                    div.text_color(theme.foreground.disabled())
                                                        .child(
                                                            item.meta
                                                                .track_number
                                                                .map(|track_number| {
                                                                    track_number.to_string()
                                                                })
                                                                .unwrap_or("-".to_string()),
                                                        )
                                                },
                                            )),
                                    )
                                    .child(item.meta.get_title())
                                    .on_click(move |_, _, cx| {
                                        // Jump to this track
                                        let play_queue =
                                            cx.global_mut::<lthebeat::play_queue::PlayQueue>();
                                        play_queue.skip_to_item(item_entity.clone());
                                    })
                                    .into_any_element()
                            }
                            DisplayQueueItem::GroupHeader(item_entity) => {
                                let item = item_entity.read(cx);

                                div()
                                    .id(ElementId::from(i))
                                    .flex()
                                    .gap(px(3.))
                                    .child(div().size(px(48.)).bg(rgb(0xFF0000)))
                                    .child(
                                        div()
                                            .flex()
                                            .flex_col()
                                            .gap(px(3.))
                                            .child(item.meta.album.clone().unwrap_or(
                                                tr!("UNKNOWN_ALBUM", "Unknown Album").into(),
                                            ))
                                            .child(
                                                div()
                                                    .text_color(theme.foreground.disabled())
                                                    .child(
                                                        item.meta.artist.clone().unwrap_or(
                                                            tr!("UNKNOWN_ARTIST", "Unknown Artist")
                                                                .into(),
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
                                    .into_any_element()
                            }
                        }
                    })
                        .with_sizing_behavior(Infer)
                        .h_full(),
                ),
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
