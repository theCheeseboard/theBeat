mod actions;
mod main_surface;
mod main_window;
mod play_queue;
mod track_metadata;
mod transport_controls;
mod views;

use crate::actions::{
    OpenFileAction, OpenUrlAction, PlayPauseAction, SkipNextAction, SkipPreviousAction,
    register_actions,
};
use crate::main_window::MainWindow;
use cntp_i18n::{I18N_MANAGER, tr, tr_load};
use cntp_icon_tool_macros::application_icon;
use contemporary::application::{ApplicationLink, Details, License, new_contemporary_application};
use contemporary::macros::application_details;
use contemporary::setup::{Contemporary, ContemporaryMenus, setup_contemporary};
use contemporary::window::contemporary_window_options;
use gpui::{App, Bounds, Menu, MenuItem, WindowBounds, WindowOptions, px, size};
use lthebeat::audio_library::database::Database;
use lthebeat::audio_processing::audio_controller::AudioController;
use lthebeat::audio_processing::audio_pipeline::plug;
use lthebeat::audio_processing::output_drivers::OutputDevice;
use lthebeat::audio_processing::output_drivers::cpal_driver::cpal_default_output_device;
use lthebeat::play_queue::PlayQueue;
use smol_macros::main;
use std::rc::Rc;

fn mane() {
    application_icon!("../dist/baseicon.svg");

    new_contemporary_application().run(|cx: &mut App| {
        I18N_MANAGER.write().unwrap().load_source(tr_load!());
        lthebeat::install_translations();
        let bounds = Bounds::centered(None, size(px(800.0), px(600.0)), cx);

        let mut play_queue = PlayQueue::new(cx);
        let mut audio_controller = AudioController::new(cx);

        plug(play_queue.open_faucet(), audio_controller.sink());

        cx.set_global(play_queue);
        cx.set_global(audio_controller);

        let default_window_options = contemporary_window_options(cx, "theBeat".into());
        register_actions(cx);
        cx.open_window(
            WindowOptions {
                window_bounds: Some(WindowBounds::Windowed(bounds)),
                ..default_window_options
            },
            |_, cx| {
                let window = MainWindow::new(cx);
                let weak_window = window.downgrade();
                let weak_windew = window.downgrade();

                cx.on_action(move |_: &OpenUrlAction, cx| {
                    weak_windew.upgrade().unwrap().update(cx, |window, cx| {
                        window.url_dialog_open(true);
                        cx.notify()
                    })
                });

                setup_contemporary(
                    cx,
                    Contemporary {
                        details: Details {
                            generatable: application_details!(),
                            copyright_holder: "Victor Tran",
                            copyright_year: "2025",
                            application_version: "5.0",
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
                            menus: vec![
                                Menu {
                                    name: tr!("MENU_FILE", "File").into(),
                                    items: vec![
                                        MenuItem::action(tr!("FILE_OPEN", "Open"), OpenFileAction),
                                        MenuItem::action(
                                            tr!("FILE_OPEN_URL", "Open URL"),
                                            OpenUrlAction,
                                        ),
                                    ],
                                },
                                Menu {
                                    name: tr!("MENU_PLAYBACK", "Playback").into(),
                                    items: vec![
                                        MenuItem::action(
                                            tr!("PLAYBACK_PLAY_PAUSE", "Play/Pause"),
                                            PlayPauseAction,
                                        ),
                                        MenuItem::separator(),
                                        MenuItem::action(
                                            tr!("PLAYBACK_SKIP_PREVIOUS", "Skip Back"),
                                            SkipPreviousAction,
                                        ),
                                        MenuItem::action(
                                            tr!("PLAYBACK_SKIP_NEXT", "Skip Next"),
                                            SkipNextAction,
                                        ),
                                    ],
                                },
                            ],
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

                let database = smol::block_on(Database::new(cx)).unwrap();
                database.start_scan(cx);
                cx.set_global(database);

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
