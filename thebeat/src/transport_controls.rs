use crate::actions::{SkipNextAction, SkipPreviousAction};
use contemporary::components::button::button;
use contemporary::components::icon::icon;
use contemporary::components::layer::layer;
use contemporary::components::slider::slider;
use contemporary::platform_support::platform_settings::PlatformSettings;
use contemporary::transition::float_transition_element::TransitionExt;
use gpui::{
    Action, Animation, App, IntoElement, ParentElement, Refineable, RenderOnce, StyleRefinement,
    Styled, Window, div, px, rgb,
};
use lthebeat::play_queue::PlayQueue;

#[derive(IntoElement)]
pub struct TransportControls {
    style: StyleRefinement,
}

pub fn transport_controls() -> TransportControls {
    TransportControls {
        style: Default::default(),
    }
}

impl RenderOnce for TransportControls {
    fn render(self, window: &mut Window, cx: &mut App) -> impl IntoElement {
        let play_queue = cx.global::<PlayQueue>();
        let platform_settings = cx.global::<PlatformSettings>();

        let mut div = layer()
            .flex()
            .flex_col()
            .gap(px(4.))
            .p(px(10.))
            .child(
                div()
                    .flex()
                    .items_center()
                    .gap(px(4.))
                    // Album Art
                    .child(div().size(px(48.)).bg(rgb(0xFF0000)))
                    // Text
                    .child(
                        div()
                            .flex()
                            .flex_col()
                            .flex_grow()
                            .child(div().text_size(px(18.)).child("Something playing"))
                            .child(div().child("Something playing")),
                    )
                    // TODO: Volume
                    .child(div())
                    .child(
                        button("shuffle-button")
                            .flat()
                            .child(icon("media-playlist-shuffle".into())),
                    )
                    .child(
                        button("repeat-button")
                            .flat()
                            .child(icon("media-repeat-single".into())),
                    )
                    .child(
                        button("skip-back-button")
                            .flat()
                            .child(icon("media-skip-backward".into()))
                            .on_click(|_, window, cx| {
                                window.dispatch_action(SkipPreviousAction.boxed_clone(), cx)
                            }),
                    )
                    .child(
                        button("play-pause-button")
                            .flat()
                            .child(icon("media-playback-pause".into()).size(32.)),
                    )
                    .child(
                        button("skip-forward-button")
                            .flat()
                            .child(icon("media-skip-forward".into()))
                            .on_click(|_, window, cx| {
                                window.dispatch_action(SkipNextAction.boxed_clone(), cx)
                            }),
                    ),
            )
            .child(
                div()
                    .flex()
                    .gap(px(4.))
                    // Elapsed
                    .child("00:00")
                    .child(slider("seek-slider").h(px(24.)).flex_grow())
                    // Total
                    .child("00:00"),
            );
        div.style().refine(&self.style);

        div.with_transition(
            "transport-controls-transition",
            if play_queue.shown_items.is_empty() {
                0.
            } else {
                96.
            },
            Animation::new(platform_settings.animation_duration),
            |this, value| this.h(px(value)),
        )
    }
}

impl Styled for TransportControls {
    fn style(&mut self) -> &mut StyleRefinement {
        &mut self.style
    }
}
