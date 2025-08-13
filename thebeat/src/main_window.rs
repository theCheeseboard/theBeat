use crate::actions::OpenUrlAction;
use crate::main_surface::MainSurface;
use cntp_i18n::tr;
use contemporary::about_surface::about_surface;
use contemporary::components::dialog_box::{StandardButton, dialog_box};
use contemporary::components::text_field::TextField;
use contemporary::window::contemporary_window;
use gpui::http_client::Url;
use gpui::prelude::FluentBuilder;
use gpui::{
    App, AppContext, Context, Entity, InteractiveElement, IntoElement, ParentElement, Render,
    Styled, Window, div, px,
};
use lthebeat::audio_processing::audio_controller::GlobalAudioController;

pub struct MainWindow {
    main_surface: Entity<MainSurface>,
    is_about_surface_open: bool,
    is_url_dialog_open: bool,
    url_text_field: Entity<TextField>,
}

impl MainWindow {
    pub fn new(cx: &mut App) -> Entity<MainWindow> {
        cx.new(|cx| MainWindow {
            main_surface: MainSurface::new(cx),
            is_about_surface_open: false,
            is_url_dialog_open: false,
            url_text_field: TextField::new(
                cx,
                "url_text_field",
                "".into(),
                tr!("URL_TEXT_FIELD_PLACEHOLDER", "URL").into(),
            ),
        })
    }

    pub fn about_surface_open(&mut self, is_open: bool) -> &Self {
        self.is_about_surface_open = is_open;
        self
    }
    
    pub fn url_dialog_open(&mut self, is_open: bool) -> &Self {
        self.is_url_dialog_open = is_open;
        self
    }
}

impl Render for MainWindow {
    fn render(&mut self, _window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        contemporary_window()
            .child(self.main_surface.clone())
            .when(self.is_about_surface_open, |w| {
                w.child(about_surface().on_back_click(cx.listener(|this, _, _, cx| {
                    this.is_about_surface_open = false;
                    cx.notify();
                })))
            })
            .child(
                dialog_box("open_url_dialog_box")
                    .visible(self.is_url_dialog_open)
                    .title(tr!("URL_OPEN_TITLE", "Open URL").into())
                    .content(
                        div()
                            .flex()
                            .flex_col()
                            .w(px(500.))
                            .gap(px(12.))
                            .child(tr!("URL_OPEN_TEXT", "Enter the URL to open"))
                            .child(self.url_text_field.clone()),
                    )
                    .standard_button(
                        StandardButton::Cancel,
                        cx.listener(|this, _, _, cx| {
                            this.is_url_dialog_open = false;
                            cx.notify();
                        }),
                    )
                    .standard_button(
                        StandardButton::Ok,
                        cx.listener(|this, _, _, cx| {
                            let text_field = this.url_text_field.read(cx);
                            let current_text = text_field.current_text(cx);
                            match Url::parse(current_text.to_string().as_str()) {
                                Ok(url) => {
                                    let global_audio_controller =
                                        cx.global::<GlobalAudioController>();
                                    global_audio_controller.audio_controller.play_url(url);
                                    this.is_url_dialog_open = false;
                                    cx.notify();
                                }
                                Err(_) => {
                                    // TODO: error flash
                                }
                            }
                        }),
                    ),
            )
    }
}
