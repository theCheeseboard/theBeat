use cntp_i18n::tr;
use contemporary::components::grandstand::grandstand;
use contemporary::components::layer::layer;
use gpui::ListSizingBehavior::Infer;
use gpui::{
    App, ElementId, InteractiveElement, IntoElement, ListAlignment, ListState, ParentElement,
    Refineable, RenderOnce, StatefulInteractiveElement, StyleRefinement, Styled, Window, div, list,
    px,
};

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

        let play_queue = cx.global::<lthebeat::play_queue::PlayQueue>();
        let list_state = list_state.read(cx);
        if play_queue.shown_items.len() != list_state.item_count() {
            list_state.reset(play_queue.shown_items.len());
        }

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
                    list(list_state.clone(), |i, _, cx| {
                        let play_queue = cx.global::<lthebeat::play_queue::PlayQueue>();
                        let item_entity = play_queue.shown_items.get(i).unwrap().clone();
                        let item = item_entity.read(cx);

                        div()
                            .id(ElementId::from(i))
                            .child(item.url.to_string())
                            .on_click(move |_, _, cx| {
                                // Jump to this track
                                let play_queue = cx.global_mut::<lthebeat::play_queue::PlayQueue>();
                                play_queue.skip_to_item(item_entity.clone());
                            })
                            .into_any_element()
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
