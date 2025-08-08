use crate::main_surface::MainSurface;
use contemporary::about_surface::about_surface;
use contemporary::window::contemporary_window;
use gpui::prelude::FluentBuilder;
use gpui::{App, AppContext, Context, Entity, IntoElement, ParentElement, Render, Window};

pub struct MainWindow {
    main_surface: Entity<MainSurface>,
    is_about_surface_open: bool,
}

impl MainWindow {
    pub fn new(cx: &mut App) -> Entity<MainWindow> {
        cx.new(|cx| MainWindow {
            main_surface: MainSurface::new(cx),
            is_about_surface_open: false,
        })
    }

    pub fn about_surface_open(&mut self, is_open: bool) -> &Self {
        self.is_about_surface_open = is_open;
        self
    }
}

impl Render for MainWindow {
    fn render(&mut self, _window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        contemporary_window().child(self.main_surface.clone()).when(
            self.is_about_surface_open,
            |w| {
                w.child(about_surface().on_back_click(cx.listener(|this, _, _, cx| {
                    this.is_about_surface_open = false;
                    cx.notify();
                })))
            },
        )
    }
}
