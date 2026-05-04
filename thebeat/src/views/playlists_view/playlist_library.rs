use cntp_i18n::tr;
use contemporary::components::admonition::AdmonitionSeverity;
use contemporary::components::button::button;
use contemporary::components::dialog_box::{StandardButton, dialog_box};
use contemporary::components::grandstand::grandstand;
use contemporary::components::icon::icon;
use contemporary::components::icon_text::icon_text;
use contemporary::components::interstitial::interstitial;
use contemporary::components::spinner::spinner;
use contemporary::components::text_field::TextField;
use contemporary::components::toast::Toast;
use contemporary::styling::theme::Theme;
use gpui::private::anyhow;
use gpui::{
    App, AppContext, AsyncApp, BorrowAppContext, Context, Entity, InteractiveElement, IntoElement,
    ParentElement, Render, StatefulInteractiveElement, Styled, Subscription, WeakEntity, Window,
    div, px,
};
use lthebeat::audio_library::album::Album;
use lthebeat::audio_library::database::Database;
use lthebeat::audio_library::database_query::DatabaseQuery;
use lthebeat::audio_library::playlist::Playlist;
use std::cell::RefCell;
use std::rc::Rc;

pub struct PlaylistLibrary {
    database_subscription: Subscription,
    playlist_query: Option<Rc<RefCell<anyhow::Result<DatabaseQuery<Playlist>>>>>,
    on_playlist_click: Rc<Box<dyn Fn(&Entity<Playlist>, &mut Window, &mut App) + 'static>>,

    create_playlist_dialog_open: bool,
    create_playlist_name: Entity<TextField>,
}

impl PlaylistLibrary {
    pub fn new(
        on_playlist_click: Box<dyn Fn(&Entity<Playlist>, &mut Window, &mut App) + 'static>,
        cx: &mut Context<Self>,
    ) -> Self {
        // The database is set as a global after the window is initialized, so this will
        // always run after the window is initialized.
        let database_subscription =
            cx.observe_global::<Database>(|_, cx: &mut Context<PlaylistLibrary>| {
                let database = cx.global::<Database>();
                let query = database.query_all_playlists();

                cx.spawn(
                    async move |playlists_view: WeakEntity<Self>, cx: &mut AsyncApp| {
                        let playlist_query = query.await;
                        playlists_view
                            .update(cx, |playlists_view, cx| {
                                playlists_view.playlist_query =
                                    Some(Rc::new(RefCell::new(playlist_query)));
                                cx.notify();
                            })
                            .unwrap();
                    },
                )
                .detach();
            });

        PlaylistLibrary {
            database_subscription,
            playlist_query: None,
            on_playlist_click: Rc::new(on_playlist_click),
            create_playlist_dialog_open: false,
            create_playlist_name: cx.new(|cx| TextField::new("create-dialog-name", cx)),
        }
    }
}

impl Render for PlaylistLibrary {
    fn render(&mut self, window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let theme = cx.global::<Theme>();

        let playlist_click_handler = self.on_playlist_click.clone();

        div()
            .bg(theme.background)
            .w_full()
            .h_full()
            .flex()
            .flex_col()
            .child(
                grandstand("playlists-grandstand")
                    .text(tr!("LIBRARY_PLAYLISTS_TITLE", "Playlists"))
                    .pt(px(36.))
                    .child(
                        button("playlist-create-button")
                            .child(icon("list-add"))
                            .flat()
                            .on_click(cx.listener(|this, _, _, cx| {
                                this.create_playlist_dialog_open = true;
                                cx.notify();
                            })),
                    ),
            )
            .child(match self.playlist_query.as_mut() {
                Some(playlist_query) => {
                    let mut playlists_query = playlist_query.borrow_mut();
                    match playlists_query.as_mut() {
                        Ok(playlists_query) => playlists_query
                            .iter(cx)
                            .enumerate()
                            .collect::<Vec<_>>()
                            .into_iter()
                            .fold(
                                div()
                                    .id("playlist-list")
                                    .flex()
                                    .flex_wrap()
                                    .justify_around()
                                    .gap(px(24.))
                                    .overflow_y_scroll(),
                                |david, (i, playlist)| {
                                    let playlist_clone = playlist.clone();
                                    let playlist_click_handler = playlist_click_handler.clone();

                                    let playlist = playlist.read(cx);
                                    david
                                        .child(div().id(i).child(match playlist {
                                            Playlist::Ok { name, .. } => div().child(name.clone()),
                                            _ => div().child("..."),
                                        }))
                                        .on_click(move |_, window, cx| {
                                            playlist_click_handler.clone()(
                                                &playlist_clone,
                                                window,
                                                cx,
                                            );
                                        })
                                },
                            )
                            .into_any_element(),
                        Err(_) => interstitial()
                            .w_full()
                            .h_full()
                            .icon("view-media-playlist")
                            .title(tr!("LIBRARY_PLAYLISTS_ERROR", "Unable to load playlists"))
                            .message(tr!("LIBRARY_CORRUPT_ERROR_MESSAGE"))
                            .child(
                                button("tracks-corrupt-erase-button")
                                    .child(icon_text("view-refresh", tr!("LIBRARY_ERASE")))
                                    .destructive()
                                    .on_click(cx.listener(|_, _, _, cx| {
                                        cx.update_global::<Database, ()>(|database, cx| {
                                            database.erase(cx);
                                            database.start_scan(cx);
                                        });
                                    })),
                            )
                            .into_any_element(),
                    }
                }
                _ => div().child(spinner()).into_any_element(),
            })
            .child(
                dialog_box("create-playlist-dialog-box")
                    .visible(self.create_playlist_dialog_open)
                    .title(tr!("PLAYLIST_CREATE_TITLE", "Create Playlist"))
                    .content(
                        div()
                            .flex()
                            .flex_col()
                            .w(px(500.))
                            .gap(px(12.))
                            .child(tr!(
                                "PLAYLIST_CREATE_PROMPT",
                                "What do you want to call this playlist?"
                            ))
                            .child(self.create_playlist_name.clone()),
                    )
                    .standard_button(
                        StandardButton::Cancel,
                        cx.listener(|this, _, _, cx| {
                            this.create_playlist_dialog_open = false;
                            cx.notify();
                        }),
                    )
                    .button(
                        button("create-button")
                            .child(icon_text(
                                "list-add",
                                tr!("PLAYLIST_CREATE_BUTTON", "Create"),
                            ))
                            .on_click(cx.listener(|this, _, window, cx| {
                                cx.update_global::<Database, _>(|db, cx| {
                                    let playlist_name =
                                        this.create_playlist_name.read(cx).text().to_string();
                                    match smol::block_on(db.create_playlist(&playlist_name, cx)) {
                                        Ok(playlist_id) => {
                                            this.create_playlist_dialog_open = false;
                                        }
                                        Err(e) => {
                                            let error_message = format!("{e}");
                                            Toast::new()
                                                .title(
                                                    &tr!(
                                                        "PLAYLIST_CREATE_ERROR_TITLE",
                                                        "Playlist not created"
                                                    )
                                                    .to_string(),
                                                )
                                                .body(&error_message)
                                                .severity(AdmonitionSeverity::Error)
                                                .post(window, cx)
                                        }
                                    }
                                });
                                cx.notify();
                            })),
                    ),
            )
    }
}
