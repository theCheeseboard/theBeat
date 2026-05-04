use cntp_i18n::tr;
use contemporary::components::button::button;
use contemporary::components::checkbox::{CheckState, CheckedChangeEvent, checkbox};
use contemporary::components::constrainer::constrainer;
use contemporary::components::dialog_box::{StandardButton, dialog_box};
use contemporary::components::grandstand::grandstand;
use contemporary::components::icon::icon;
use contemporary::components::icon_text::icon_text;
use contemporary::components::layer::layer;
use contemporary::components::subtitle::subtitle;
use contemporary::icon_tool::Url;
use contemporary::surface::surface;
use directories::UserDirs;
use gpui::prelude::FluentBuilder;
use gpui::{
    App, AppContext, AsyncApp, BorrowAppContext, ClickEvent, Context, Element, Entity,
    InteractiveElement, IntoElement, ListSizingBehavior, ParentElement, PathPromptOptions, Render,
    Styled, WeakEntity, Window, div, px, uniform_list,
};
use lthebeat::audio_library::database::Database;
use lthebeat::play_queue::PlayQueue;
use lthebeat::play_queue::media_item::MediaItem;
use std::rc::Rc;

pub struct DatabaseSetupSurface {
    on_back_button_click: Rc<Box<dyn Fn(&ClickEvent, &mut Window, &mut App) + 'static>>,
    paths: Vec<String>,
    error_dialog_open: bool,
}

impl DatabaseSetupSurface {
    pub fn new(
        on_back_button_click: Box<dyn Fn(&ClickEvent, &mut Window, &mut App) + 'static>,
        cx: &mut App,
    ) -> Entity<Self> {
        cx.new(|cx| {
            cx.observe_global::<Database>(
                |database_setup_surface: &mut DatabaseSetupSurface, cx| {
                    let database = cx.global::<Database>();
                    database_setup_surface.paths = smol::block_on(database.get_scan_directories());
                    cx.notify();
                },
            )
            .detach();

            DatabaseSetupSurface {
                on_back_button_click: Rc::new(on_back_button_click),
                paths: Vec::new(),
                error_dialog_open: false,
            }
        })
    }
}

impl Render for DatabaseSetupSurface {
    fn render(&mut self, _: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let back_button_click = self.on_back_button_click.clone();
        let back_button_click_2 = self.on_back_button_click.clone();

        let music_dir = UserDirs::new().and_then(|user_dir| {
            user_dir
                .audio_dir()
                .and_then(|path| path.to_str().map(|str| str.to_string()))
        });

        let other_folder_list_contents: Vec<_> = self
            .paths
            .iter()
            .filter(|path| match &music_dir {
                Some(music_dir) => *path != music_dir,
                None => true,
            })
            .cloned()
            .collect();

        surface().child(
            div()
                .flex()
                .flex_col()
                .gap(px(4.))
                .w_full()
                .h_full()
                .child(
                    grandstand("database-setup-grandstand")
                        .text(tr!("DATABASE_SETUP_TITLE", "Library Setup"))
                        .pt(px(36.))
                        .on_back_click(cx.listener(move |this, event, window, cx| {
                            cx.update_global::<Database, ()>(|database, cx| {
                                if smol::block_on(database.set_scan_directories(&this.paths, cx))
                                    .is_ok()
                                {
                                    database.start_scan(cx);
                                    back_button_click(event, window, cx);
                                } else {
                                    this.error_dialog_open = true;
                                }
                            });
                            cx.notify();
                        })),
                )
                .child(
                    constrainer("database-setup-constrainer")
                        .flex()
                        .flex_col()
                        .w_full()
                        .p(px(8.))
                        .gap(px(8.))
                        .child(
                            layer()
                                .flex()
                                .flex_col()
                                .p(px(8.))
                                .w_full()
                                .child(subtitle(tr!("DATABASE_SETUP_SCAN_FOLDERS", "Scan Folders")))
                                .child(
                                    div()
                                        .flex()
                                        .flex_col()
                                        .gap(px(8.))
                                        .child(tr!(
                                            "DATABASE_SETUP_SCAN_FOLDERS_DESCRIPTION",
                                            "theBeat will scan these folders for music on startup"
                                        ))
                                        .child(
                                            checkbox("music-folder-checkbox").label(tr!(
                                                "DATABASE_SETUP_SCAN_MUSIC_FOLDER",
                                                "Scan Music folder"
                                            ))
                                            .when_some(music_dir, |checkbox, music_dir| {
                                                let music_dir_2 = music_dir.clone();
                                                checkbox
                                                    .when(self.paths.contains(&music_dir), |checkbox| checkbox.checked())
                                                    .on_checked_changed(cx.listener(
                                                        move |this, event: &CheckedChangeEvent, _, cx| {
                                                            if event.check_state == CheckState::On {
                                                                this.paths.push(music_dir_2.clone())
                                                            } else {
                                                                this.paths
                                                                    .retain(|path| path != &music_dir_2)
                                                            }

                                                            cx.notify()
                                                        },
                                                    ))
                                            }),
                                        ),
                                ),
                        )
                        .child(
                            layer()
                                .flex()
                                .flex_col()
                                .p(px(8.))
                                .w_full()
                                .child(subtitle(tr!(
                                    "DATABASE_SETUP_SCAN_OTHER_FOLDERS",
                                    "Other Scan Folders"
                                )))
                                .child(div().flex().flex_col().gap(px(8.)).child(tr!(
                                    "DATABASE_SETUP_OTHER_SCAN_FOLDERS_DESCRIPTION",
                                    "You can add other folders here and theBeat will also scan \
                                    these folders for music on startup."
                                )))
                                .child(
                                    uniform_list(
                                        "other-folder-list",
                                        other_folder_list_contents.len(),
                                        {
                                            let weak_this = cx.entity().downgrade();
                                            move |range, _, cx| {
                                                range
                                                    .map(|index| {
                                                        let weak_this = weak_this.clone();
                                                        let path =
                                                            &other_folder_list_contents[index];
                                                        div()
                                                            .id(index)
                                                            .flex()
                                                            .w_full()
                                                            .items_center()
                                                            .p(px(2.))
                                                            .gap(px(8.))
                                                            .child(div().child(path.clone()).flex_grow())
                                                            .child(
                                                                button("remove-button")
                                                                    .flat()
                                                                    .child(icon(
                                                                        "list-remove",
                                                                    ))
                                                                    .on_click(move |_, _, cx| {
                                                                        weak_this
                                                                            .update(
                                                                                cx,
                                                                                |this, cx| {
                                                                                    this.paths
                                                                                        .remove(
                                                                                            index,
                                                                                        );
                                                                                    cx.notify()
                                                                                },
                                                                            )
                                                                            .unwrap()
                                                                    }),
                                                            )
                                                    })
                                                    .collect()
                                            }
                                        },
                                    )
                                    .with_sizing_behavior(ListSizingBehavior::Infer),
                                )
                                .child(
                                    div().flex().child(div().flex_grow()).child(
                                        button("add-other-folder").child(icon_text(
                                            "list-add",
                                            tr!(
                                                "DATABASE_SETUP_OTHER_SCAN_FOLDERS_ADD",
                                                "Browse for folder..."
                                            ),
                                        ))
                                        .on_click(cx.listener(|_, _, _, cx| {
                                            let future = cx.prompt_for_paths(PathPromptOptions {
                                                files: false,
                                                directories: true,
                                                multiple: false,
                                                prompt: Some(tr!("DATABASE_SETUP_ADD_TO_LIBRARY", "Add to Library").into()),
                                            });
                                            cx.spawn(async |weak_this: WeakEntity<Self>, cx: &mut AsyncApp| {
                                                let result = future.await;
                                                if let Ok(result) = result && let Some(this) = weak_this.upgrade() {
                                                    cx.update_entity(&this, |this, cx| {
                                                        if let Ok(Some(paths)) = result {
                                                            for path in paths {
                                                                this.paths.push(path.to_str().unwrap().to_string())
                                                            }
                                                        }

                                                        cx.notify();
                                                    });
                                                }
                                            })
                                                .detach()
                                        })),
                                    ),
                                ),
                        ),
                )
                .child(
                    dialog_box("database-setup-error")
                        .title(
                            tr!(
                                "DATABASE_SETUP_SUBMIT_ERROR_TITLE",
                                "Unable to save library setup"
                            ),
                        )
                        .content(tr!(
                            "DATABASE_SETUP_SUBMIT_ERROR_MESSAGE",
                            "There was a problem saving your library setup."
                        ))
                        .standard_button(
                            StandardButton::Cancel,
                            cx.listener(|this, _, _, cx| {
                                this.error_dialog_open = false;
                                cx.notify()
                            }),
                        )
                        .button(
                            button("database-setup-error-ignore")
                                .destructive()
                                .child(tr!(
                                    "DATABASE_SETUP_SUBMIT_IGNORE_DISCARD",
                                    "Ignore and discard changes"
                                ))
                                .on_click(cx.listener(move |this, event, window, cx| {
                                    let database = cx.global::<Database>();
                                    this.paths = smol::block_on(database.get_scan_directories());
                                    back_button_click_2(event, window, cx);
                                })),
                        )
                        .visible(self.error_dialog_open),
                ),
        )
    }
}
