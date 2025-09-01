use cntp_i18n::tr;
use contemporary::components::grandstand::grandstand;
use contemporary::styling::theme::Theme;
use gpui::{
    App, AppContext, Context, Entity, IntoElement, ParentElement, Render, Styled, Window, div, px,
};

pub struct TracksView {}

impl TracksView {
    pub fn new(cx: &mut App) -> Entity<Self> {
        cx.new(|_| TracksView {})
    }
}

impl Render for TracksView {
    fn render(&mut self, window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let theme = cx.global::<Theme>();

        div()
            .bg(theme.background)
            .w_full()
            .h_full()
            .flex()
            .flex_col()
            .child(
                grandstand("tracks-grandstand")
                    .text(tr!("LIBRARY_TRACKS_TITLE", "Tracks in Library"))
                    .pt(px(36.)),
            )
    }
}
