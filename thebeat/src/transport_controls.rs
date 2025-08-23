use crate::actions::{SkipNextAction, SkipPreviousAction};
use crate::track_metadata::TrackMetadata;
use contemporary::components::button::button;
use contemporary::components::icon::icon;
use contemporary::components::layer::layer;
use contemporary::components::slider::slider;
use contemporary::easing::ease_out_cubic;
use contemporary::platform_support::platform_settings::PlatformSettings;
use contemporary::transition::float_transition_element::TransitionExt;
use gpui::prelude::FluentBuilder;
use gpui::{
    Action, Animation, App, BorrowAppContext, IntoElement, ParentElement, Refineable, RenderOnce,
    StyleRefinement, Styled, Window, div, px, rgb,
};
use lthebeat::audio_processing::audio_controller::AudioController;
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
    fn render(self, _: &mut Window, cx: &mut App) -> impl IntoElement {
        let play_queue = cx.global::<PlayQueue>();
        let platform_settings = cx.global::<PlatformSettings>();
        let audio_controller = cx.global::<AudioController>();

        let meta = audio_controller.current_metadata();
        let current_time = audio_controller.current_time();
        let title = meta.get_title();
        let supplementary = meta.supplementary_text();

        let mut div = layer()
            .flex()
            .flex_col()
            .gap(px(4.))
            .p(px(10.))
            .overflow_hidden()
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
                            .child(div().text_size(px(18.)).child(title))
                            .child(div().child(supplementary)),
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
                            .when_else(
                                audio_controller.is_playing(),
                                |button| {
                                    button.child(icon("media-playback-pause".into()).size(32.))
                                },
                                |button| {
                                    button.child(icon("media-playback-start".into()).size(32.))
                                },
                            )
                            .on_click(|_, _, cx| {
                                cx.update_global::<AudioController, ()>(|audio_controller, _| {
                                    audio_controller.play_pause()
                                })
                            }),
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
                    .child(
                        current_time
                            .map(|d| {
                                let secs = d.as_secs();
                                format!("{:02}:{:02}", secs / 60, secs % 60)
                            })
                            .unwrap_or("??:??".to_string()),
                    )
                    .child(
                        slider("seek-slider")
                            .h(px(24.))
                            .flex_grow()
                            .when_some(meta.duration, |slider, duration| {
                                slider.when_some(current_time, |slider, current_time| {
                                    slider
                                        .value(current_time.as_millis() as u32)
                                        .max_value(duration.as_millis() as u32)
                                })
                            })
                            .when_none(&meta.duration, |slider| slider.disabled())
                            .when_none(&current_time, |slider| slider.disabled()),
                    )
                    // Total
                    .child(
                        meta.duration
                            .map(|d| {
                                let secs = d.as_secs();
                                format!("{:02}:{:02}", secs / 60, secs % 60)
                            })
                            .unwrap_or("∞".to_string()),
                    ),
            );
        div.style().refine(&self.style);

        div.with_transition(
            "transport-controls-transition",
            if play_queue.shown_items.is_empty() {
                0.
            } else {
                96.
            },
            Animation::new(platform_settings.animation_duration).with_easing(ease_out_cubic),
            |this, value| this.h(px(value)).max_h(px(value)),
        )
    }
}

impl Styled for TransportControls {
    fn style(&mut self) -> &mut StyleRefinement {
        &mut self.style
    }
}
