use cntp_i18n::tr;
use contemporary::components::grandstand::grandstand;
use contemporary::components::interstitial::interstitial;
use contemporary::components::layer::layer;
use contemporary::components::pager::lift_animation::LiftAnimation;
use contemporary::components::pager::{PageNumber, pager};
use contemporary::components::scrollbar::SelfScrollable;
use contemporary::styling::theme::{Theme, ThemeStorage};
use gpui::prelude::FluentBuilder;
use gpui::{
    App, AppContext, Context, ElementId, Entity, InteractiveElement, IntoElement, ParentElement,
    Render, StatefulInteractiveElement, Styled, Window, div, px, uniform_list,
};
use lthebeat::other_sources::OtherSourcesManager;
use std::ops::Range;

pub struct OtherSourcesView {
    selected_item: Option<ElementId>,
}

impl OtherSourcesView {
    pub fn new(cx: &mut App) -> Entity<Self> {
        cx.new(|_| OtherSourcesView {
            selected_item: None,
        })
    }
}

impl Render for OtherSourcesView {
    fn render(&mut self, window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let theme = cx.theme();

        let sources = cx.global::<OtherSourcesManager>().sources();
        let ids = sources.iter().map(|source| source.id());

        if sources.is_empty() {
            div()
                .bg(theme.background)
                .w_full()
                .h_full()
                .flex()
                .flex_col()
                .child(
                    interstitial()
                        .size_full()
                        .title(tr!(
                            "OTHER_SOURCES_UNAVAILABLE_TITLE",
                            "No other sources available"
                        ))
                        .message(tr!(
                            "OTHER_SOURCES_UNAVAILABLE_MESSAGE",
                            "There's nothing else to play right now."
                        )),
                )
        } else {
            let sources_pager = sources.iter().fold(
                pager(
                    "other-sources-pager",
                    self.selected_item
                        .as_ref()
                        .map(|selected_item| {
                            ids.enumerate()
                                .find(|(_, id)| selected_item == id)
                                .map(|(idx, _)| idx + 1)
                        })
                        .flatten()
                        .unwrap_or_default(),
                )
                .animation(LiftAnimation::new())
                .flex_grow(1.)
                .page(
                    interstitial()
                        .bg(theme.background)
                        .title(tr!("OTHER_SOURCES_NOTHING_SELECTED", "Select a source"))
                        .child(tr!(
                            "OTHER_SOURCES_NOTHING_SELECTED_DESCRIPTION",
                            "Select a source to find audio to play"
                        ))
                        .size_full(),
                ),
                |pager, source| {
                    pager.page(
                        div()
                            .bg(theme.background)
                            .size_full()
                            .child(source.render()),
                    )
                },
            );

            div()
                .bg(theme.background)
                .w_full()
                .h_full()
                .flex()
                .child(
                    layer()
                        .w(px(300.))
                        .flex()
                        .flex_col()
                        .child(
                            grandstand("other-sources-sidebar-grandstand")
                                .text(tr!("OTHER_SOURCES_TITLE", "Other Sources"))
                                .pt(px(36.)),
                        )
                        .child(
                            div().flex_grow(1.).p(px(2.)).child(
                                uniform_list(
                                    "other-sources-list",
                                    sources.len(),
                                    cx.processor(move |this, range: Range<usize>, _, cx| {
                                        let sources = cx.global::<OtherSourcesManager>().sources();
                                        let theme = cx.theme();

                                        range
                                            .map(|i| {
                                                let source = &sources[i];
                                                let id = source.id().clone();
                                                div()
                                                    .id(id.clone())
                                                    .rounded(theme.border_radius)
                                                    .child(source.name().to_string())
                                                    .when(
                                                        this.selected_item.as_ref().is_some_and(
                                                            |selected_item| selected_item == &id,
                                                        ),
                                                        |david| david.bg(theme.button_background),
                                                    )
                                                    .on_click(cx.listener(move |this, _, _, cx| {
                                                        this.selected_item = Some(id.clone());
                                                        cx.notify()
                                                    }))
                                            })
                                            .collect()
                                    }),
                                )
                                .self_scrollable(window, cx)
                                .h_full()
                                .w_full(),
                            ),
                        ),
                )
                .child(sources_pager)
        }
    }
}
