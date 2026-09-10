use cntp_i18n::{tr, trn};
use contemporary::components::button::button;
use contemporary::components::icon_text::icon_text;
use contemporary::components::layer::layer;
use contemporary::components::subtitle::subtitle;
use gpui::prelude::FluentBuilder;
use gpui::{
    AnyElement, App, Div, InteractiveElement, IntoElement, ParentElement, RenderOnce, Styled,
    Window, div, px, uniform_list,
};

#[derive(IntoElement)]
pub struct TrackListSkeleton {
    side_area: Div,
    track_count: Option<usize>,
    main_area: AnyElement,
}

pub fn track_list_skeleton(main_area: impl IntoElement) -> TrackListSkeleton {
    TrackListSkeleton {
        side_area: div().p(px(8.)).gap(px(8.)).flex().flex_col(),
        track_count: None,
        main_area: main_area.into_any_element(),
    }
}

impl TrackListSkeleton {
    pub fn side_list_area_child(mut self, child: impl IntoElement) -> Self {
        self.side_area = self.side_area.child(child);
        self
    }

    pub fn track_count(mut self, count: usize) -> Self {
        self.track_count = Some(count);
        self
    }
}

impl RenderOnce for TrackListSkeleton {
    fn render(self, window: &mut Window, cx: &mut App) -> impl IntoElement {
        div()
            .id("track-listing")
            .flex()
            .child(self.side_area.child(div().flex_grow(1.)).when_some(
                self.track_count,
                |david, track_count| {
                    david.child(trn!(
                        "TRACK_LISTING_TRACK_COUNT",
                        "{{count}} track",
                        "{{count}} tracks",
                        count = track_count as isize
                    ))
                },
            ))
            .child(self.main_area)
            .h_full()
            .w_full()
    }
}
