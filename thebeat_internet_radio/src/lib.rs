use gpui::{
    AnyElement, App, AppContext, BorrowAppContext, Context, ElementId, Entity, IntoElement,
    ParentElement, Render, Styled, Window, div,
};
use lthebeat::other_sources::{OtherSource, OtherSourcesManager};

pub fn init(cx: &mut App) {
    cx.update_global::<OtherSourcesManager, _>(|other_sources_manager, cx| {
        other_sources_manager.push_source(Box::new(InternetRadioSource {
            element: cx.new(|cx| InternetRadioElement::new(cx)),
        }))
    });
}

pub struct InternetRadioSource {
    element: Entity<InternetRadioElement>,
}

impl OtherSource for InternetRadioSource {
    fn render(&self) -> AnyElement {
        self.element.clone().into_any_element()
    }

    fn name(&self) -> &str {
        "Internet Radio"
    }

    fn id(&self) -> ElementId {
        "internet-radio".into()
    }
}

struct InternetRadioElement {}

impl InternetRadioElement {
    pub fn new(_: &mut Context<Self>) -> Self {
        InternetRadioElement {}
    }
}

impl Render for InternetRadioElement {
    fn render(&mut self, window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        div().child("Internet Radio").size_full()
    }
}
