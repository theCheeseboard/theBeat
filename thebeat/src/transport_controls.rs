use crate::actions::{SkipNextAction, SkipPreviousAction};
use crate::track_metadata::TrackMetadata;
use contemporary::components::button::button;
use contemporary::components::icon::icon;
use contemporary::components::slider::{SliderChangeEvent, slider};
use contemporary::easing::ease_out_cubic;
use contemporary::platform_support::platform_settings::PlatformSettings;
use contemporary::styling::theme::Theme;
use contemporary::transition::float_transition_element::TransitionExt;
use gpui::prelude::FluentBuilder;
use gpui::{
    Action, Animation, App, AppContext, BorrowAppContext, Context, Div, Entity, FontFeatures,
    ImageSource, InteractiveElement, IntoElement, ParentElement, Refineable, Render, Rgba,
    StatefulInteractiveElement, StyleRefinement, Styled, Window, div, img, px, rgb,
};
use lthebeat::audio_processing::audio_controller::AudioController;
use lthebeat::play_queue::PlayQueue;
use std::sync::Arc;
use std::time::Duration;

pub struct TransportControls {
    style: StyleRefinement,
    seek_value: Option<Duration>,
    volume_hovering: bool,
}

impl TransportControls {
    pub fn new(cx: &mut App) -> Entity<TransportControls> {
        cx.new(|_| TransportControls {
            style: Default::default(),
            seek_value: None,
            volume_hovering: false,
        })
    }
}

impl Render for TransportControls {
    fn render(
        &mut self,
        _: &mut Window,
        cx: &mut Context<'_, TransportControls>,
    ) -> impl IntoElement {
        let play_queue = cx.global::<PlayQueue>();
        let platform_settings = cx.global::<PlatformSettings>();
        let audio_controller = cx.global::<AudioController>();
        let theme = cx.global::<Theme>();

        let meta = audio_controller.current_metadata();
        let current_time = audio_controller.current_time();
        let title = meta.get_title();
        let supplementary = meta.supplementary_text();

        let cover = meta
            .album_cover
            .clone()
            .and_then(|album_cover| album_cover.render_image())
            .clone();
        let average_color = meta
            .album_cover
            .and_then(|album_cover| album_cover.average_color());

        let mut div = div()
            .rounded(theme.border_radius)
            .flex()
            .flex_col()
            .gap(px(4.))
            .p(px(10.))
            .overflow_hidden()
            .when_some(average_color, |layer, average_color| {
                layer.bg(Rgba {
                    a: 0.2,
                    ..average_color
                })
            })
            .when_none(&average_color, |layer| layer.bg(theme.layer_background))
            .child(
                div()
                    .flex()
                    .items_center()
                    .gap(px(4.))
                    .max_w_full()
                    .overflow_hidden()
                    // Album Art
                    .child(
                        div()
                            .size(px(48.))
                            .when_some(cover.clone(), |div, album_cover| {
                                div.child(img(ImageSource::Render(album_cover)).h_full().w_full())
                            })
                            .when_none(&cover, |div| div.bg(rgb(0xFF0000))),
                    )
                    // Text
                    .child(
                        div()
                            .flex()
                            .flex_col()
                            .flex_grow(1.)
                            .overflow_hidden()
                            .child(div().text_size(px(18.)).text_ellipsis().child(title))
                            .child(div().text_ellipsis().child(supplementary)),
                    )
                    .child(
                        div()
                            .id("volume-container")
                            .flex()
                            .items_center()
                            .p(px(4.))
                            .mr(px(-4.))
                            .on_hover(cx.listener(|this, hovering, _, cx| {
                                this.volume_hovering = *hovering;
                                cx.notify()
                            }))
                            .child(
                                div().pr(px(4.)).child(
                                    slider("master-volume-slider")
                                        .max_value(100)
                                        .value((audio_controller.master_volume() * 100.) as u32)
                                        .h(px(16.))
                                        .on_change(|event: &SliderChangeEvent, _, cx| {
                                            cx.update_global::<AudioController, ()>(
                                                |audio_controller, _| {
                                                    audio_controller.set_master_volume(
                                                        event.new_value as f64 / 100.,
                                                    );
                                                },
                                            );
                                        })
                                        .with_transition(
                                            "master-volume-slider-transition",
                                            if self.volume_hovering { 1. } else { 0. },
                                            Animation::new(platform_settings.animation_duration)
                                                .with_easing(ease_out_cubic),
                                            |slider, progress| {
                                                slider.w(px(progress * 100.)).opacity(progress)
                                            },
                                        ),
                                ),
                            )
                            .child(
                                button("volume-button")
                                    .flat()
                                    .child(icon("audio-volume-high")),
                            ),
                    )
                    .child(
                        button("shuffle-button")
                            .flat()
                            .child(icon("media-playlist-shuffle"))
                            .checked_when(play_queue.shuffle)
                            .on_click(|_, _, cx| {
                                cx.update_global::<PlayQueue, ()>(|play_queue, cx| {
                                    play_queue.shuffle(!play_queue.shuffle, cx)
                                })
                            }),
                    )
                    .child(
                        button("repeat-button")
                            .flat()
                            .child(icon("media-repeat-single"))
                            .checked_when(play_queue.repeat_one())
                            .on_click(|_, _, cx| {
                                cx.update_global::<PlayQueue, ()>(|play_queue, cx| {
                                    play_queue.set_repeat_one(!play_queue.repeat_one(), cx)
                                })
                            }),
                    )
                    .child(
                        button("skip-back-button")
                            .flat()
                            .child(icon("media-skip-backward"))
                            .on_click(|_, window, cx| {
                                window.dispatch_action(SkipPreviousAction.boxed_clone(), cx)
                            }),
                    )
                    .child(
                        button("play-pause-button")
                            .flat()
                            .when_else(
                                audio_controller.is_playing(),
                                |button| button.child(icon("media-playback-pause").size(32.)),
                                |button| button.child(icon("media-playback-start").size(32.)),
                            )
                            .on_click(|_, _, cx| {
                                cx.update_global::<AudioController, ()>(|audio_controller, cx| {
                                    audio_controller.play_pause(cx)
                                })
                            }),
                    )
                    .child(
                        button("skip-forward-button")
                            .flat()
                            .child(icon("media-skip-forward"))
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
                    .child(tabular_numbers(
                        self.seek_value
                            .or(current_time)
                            .map(|d| {
                                let secs = d.as_secs();
                                format!("{:02}:{:02}", secs / 60, secs % 60)
                            })
                            .unwrap_or("??:??".to_string()),
                    ))
                    .child(
                        slider("seek-slider")
                            .h(px(20.))
                            .flex_grow(1.)
                            .when_some(meta.duration, |slider, duration| {
                                slider.when_some(current_time, |slider, current_time| {
                                    slider
                                        .value(current_time.as_millis() as u32)
                                        .max_value(duration.as_millis() as u32)
                                        .on_press(|_, _, cx| {
                                            cx.update_global::<AudioController, ()>(
                                                |audio_controller, cx| {
                                                    audio_controller.pause(cx);
                                                },
                                            );
                                        })
                                        .on_release(cx.listener(|this, _, _, cx| {
                                            if let Some(seek_position) = this.seek_value {
                                                cx.update_global::<PlayQueue, ()>(
                                                    |play_queue, cx| {
                                                        play_queue
                                                            .seek_to_position(seek_position, cx);
                                                    },
                                                );
                                            };
                                            cx.update_global::<AudioController, ()>(
                                                |audio_controller, cx| {
                                                    audio_controller.play(cx);
                                                },
                                            );

                                            this.seek_value = None;
                                            cx.notify();
                                        }))
                                        .on_change(cx.listener(
                                            |this, change_event: &SliderChangeEvent, _, cx| {
                                                this.seek_value = Some(Duration::from_millis(
                                                    change_event.new_value as u64,
                                                ));
                                                cx.notify();
                                            },
                                        ))
                                })
                            })
                            .when_none(&meta.duration, |slider| slider.disabled())
                            .when_none(&current_time, |slider| slider.disabled()),
                    )
                    // Total
                    .child(tabular_numbers(
                        meta.duration
                            .map(|d| {
                                let secs = d.as_secs();
                                format!("{:02}:{:02}", secs / 60, secs % 60)
                            })
                            .unwrap_or("∞".to_string()),
                    )),
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

fn tabular_numbers(text: String) -> Div {
    let mut david = div().child(text);
    let ff = &mut david.text_style().font_features;
    *ff = Some(FontFeatures(Arc::new(vec![("tnum".to_string(), 1)])));
    david
}
