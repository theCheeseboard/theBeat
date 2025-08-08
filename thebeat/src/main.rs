mod main_window;
mod main_surface;

use std::rc::Rc;
use cntp_i18n::{tr, tr_load, I18N_MANAGER};
use cntp_icon_tool_macros::application_icon;
use contemporary::application::{new_contemporary_application, ApplicationLink, Details, License};
use contemporary::macros::application_details;
use contemporary::setup::{setup_contemporary, Contemporary, ContemporaryMenus};
use contemporary::window::contemporary_window_options;
use gpui::{px, size, App, Bounds, Menu, MenuItem, Radians, WindowBounds, WindowOptions};
use smol_macros::main;
use crate::main_window::MainWindow;

fn mane() {
    application_icon!("../dist/baseicon.svg");
    new_contemporary_application().run(|cx: &mut App| {
        I18N_MANAGER.write().unwrap().load_source(tr_load!());
        let bounds = Bounds::centered(None, size(px(800.0), px(600.0)), cx);

        let default_window_options = contemporary_window_options(cx);
        cx.open_window(
            WindowOptions {
                window_bounds: Some(WindowBounds::Windowed(bounds)),
                ..default_window_options
            },
            |_, cx| {
                let window = MainWindow::new(cx);
                let weak_window = window.downgrade();

                setup_contemporary(
                    cx,
                    Contemporary {
                        details: Details {
                            generatable: application_details!(),
                            copyright_holder: "Victor Tran",
                            copyright_year: "2025",
                            application_version: "3.0",
                            license: License::Gpl3OrLater,
                            links: [
                                (
                                    ApplicationLink::FileBug,
                                    "https://github.com/vicr123/thecalculator/issues",
                                ),
                                (
                                    ApplicationLink::SourceCode,
                                    "https://github.com/vicr123/thecalculator",
                                ),
                            ]
                                .into(),
                        },
                        menus: ContemporaryMenus {
                            menus: vec![Menu {
                                name: tr!("MENU_FILE", "File").into(),
                                items: vec![
                                ],
                            }],
                            on_about: Rc::new(move |cx| {
                                weak_window.upgrade().unwrap().update(cx, |window, cx| {
                                    window.about_surface_open(true);
                                    cx.notify()
                                })
                            }),
                            on_settings: None,
                        },
                    },
                );

                window
            },
        )
            .unwrap();
        cx.activate(true);
    });
}

main! {
    async fn main() {
        mane()
    }
}
