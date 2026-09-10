use crate::actions::DatabaseSetupAction;
use crate::database_setup::database_setup_surface::DatabaseSetupSurface;
use crate::main_surface::MainSurface;
use cntp_i18n::tr;
use contemporary::about_surface::about_surface;
use contemporary::components::dialog_box::{StandardButton, dialog_box};
use contemporary::components::pager::lift_animation::LiftAnimation;
use contemporary::components::pager::pager;
use contemporary::components::text_field::TextField;
use contemporary::window::contemporary_window;
use gpui::http_client::Url;
use gpui::prelude::FluentBuilder;
use gpui::{
    App, AppContext, BorrowAppContext, Context, Entity, IntoElement, ParentElement, Render, Styled,
    Window, div, px,
};
use lthebeat::play_queue::PlayQueue;
use lthebeat::play_queue::media_item::MediaItem;
use std::any::TypeId;
use std::rc::Rc;

pub struct MainWindow {
    main_surface: Entity<MainSurface>,
    database_setup_surface: Entity<DatabaseSetupSurface>,
    current_surface: Vec<MainWindowSurface>,
    is_url_dialog_open: bool,
    url_text_field: Entity<TextField>,
}

enum MainWindowSurface {
    Main,
    DatabaseSetup,
    About,
}

impl MainWindow {
    pub fn new(cx: &mut App) -> Entity<MainWindow> {
        cx.new(|cx| {
            let setup_button_click_listener = cx.listener(|this: &mut MainWindow, _, _, cx| {
                this.current_surface.push(MainWindowSurface::DatabaseSetup);
                cx.notify()
            });
            let back_button_click_listener = cx.listener(|this: &mut MainWindow, _, _, cx| {
                this.current_surface.pop();
                cx.notify()
            });

            MainWindow {
                main_surface: MainSurface::new(Rc::new(Box::new(setup_button_click_listener)), cx),
                database_setup_surface: DatabaseSetupSurface::new(
                    Box::new(back_button_click_listener),
                    cx,
                ),
                current_surface: vec![MainWindowSurface::Main],
                is_url_dialog_open: false,
                url_text_field: cx.new(|cx| {
                    let mut text_field = TextField::new("url_text_field", cx);
                    text_field.set_placeholder(&tr!("URL_TEXT_FIELD_PLACEHOLDER", "URL"));
                    text_field
                }),
            }
        })
    }

    pub fn about_surface_open(&mut self, is_open: bool) -> &Self {
        if is_open {
            self.current_surface.push(MainWindowSurface::About);
        } else {
            self.current_surface.pop();
        }
        self
    }

    pub fn database_setup_surface_open(&mut self, is_open: bool) -> &Self {
        if is_open {
            self.current_surface.push(MainWindowSurface::DatabaseSetup);
        } else {
            self.current_surface.pop();
        }
        self
    }

    pub fn url_dialog_open(&mut self, is_open: bool) -> &Self {
        self.is_url_dialog_open = is_open;
        self
    }
}

impl Render for MainWindow {
    fn render(&mut self, window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        contemporary_window()
            .child(
                pager(
                    "main-pager",
                    match self.current_surface.last().unwrap() {
                        MainWindowSurface::Main => 0,
                        MainWindowSurface::DatabaseSetup => 1,
                        MainWindowSurface::About => 2,
                    },
                )
                .w_full()
                .h_full()
                .animation(LiftAnimation::new())
                .page(self.main_surface.clone().into_any_element())
                .page(self.database_setup_surface.clone().into_any_element())
                .page(
                    about_surface()
                        .on_back_click(cx.listener(|this, _, _, cx| {
                            this.current_surface.pop();
                            cx.notify();
                        }))
                        .into_any_element(),
                ),
            )
            .child(
                dialog_box("open_url_dialog_box")
                    .visible(self.is_url_dialog_open)
                    .title(tr!("URL_OPEN_TITLE", "Open URL"))
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
                            let current_text = text_field.text();
                            match Url::parse(current_text.to_string().as_str()) {
                                Ok(url) => {
                                    let item = cx.new(|cx| MediaItem::new(url, cx));
                                    cx.update_global::<PlayQueue, ()>(|play_queue, cx| {
                                        play_queue.add_item(item, cx);
                                    });
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
