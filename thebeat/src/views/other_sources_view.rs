use cntp_i18n::tr;
use contemporary::components::interstitial::interstitial;
use contemporary::styling::theme::Theme;
use gpui::{
    App, AppContext, Context, Entity, IntoElement, ParentElement, Render, Styled, Window, div,
};

pub struct OtherSourcesView {}

impl OtherSourcesView {
    pub fn new(cx: &mut App) -> Entity<Self> {
        cx.new(|_| OtherSourcesView {})
    }
}

impl Render for OtherSourcesView {
    fn render(&mut self, _: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let theme = cx.global::<Theme>();

        div()
            .bg(theme.background)
            .w_full()
            .h_full()
            .flex()
            .flex_col()
            .child(
                interstitial()
                    .size_full()
                    .title(
                        tr!(
                            "OTHER_SOURCES_UNAVAILABLE_TITLE",
                            "No other sources available"
                        )
                        .into(),
                    )
                    .message(
                        tr!(
                            "OTHER_SOURCES_UNAVAILABLE_MESSAGE",
                            "There's nothing else to play right now."
                        )
                        .into(),
                    ),
            )
    }
}
