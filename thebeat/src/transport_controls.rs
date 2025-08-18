use contemporary::transition::float_transition_element::TransitionExt;
use contemporary::platform_support::platform_settings::PlatformSettings;
use contemporary::components::layer::layer;
use gpui::{
    Animation, App, IntoElement, ParentElement, Refineable, RenderOnce, StyleRefinement, Styled,
    Window, div, px,
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

        let mut div = layer().child("Transport Controls");
        div.style().refine(&self.style);

        div.with_transition(
            "transport-controls-transition",
            if play_queue.shown_items.is_empty() {
                0.
            } else {
                50.
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
