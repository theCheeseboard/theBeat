use cntp_i18n::tr;
use contemporary::components::button::button;
use contemporary::components::grandstand::grandstand;
use contemporary::components::icon_text::icon_text;
use contemporary::components::interstitial::interstitial;
use contemporary::components::spinner::spinner;
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
use std::cell::RefCell;
use std::rc::Rc;

pub struct AlbumLibrary {
    database_subscription: Subscription,
    albums_query: Option<Rc<RefCell<anyhow::Result<DatabaseQuery<Album>>>>>,
    on_album_click: Rc<Box<dyn Fn(&Entity<Album>, &mut Window, &mut App) + 'static>>,
}

impl AlbumLibrary {
    pub fn new(
        on_album_click: Box<dyn Fn(&Entity<Album>, &mut Window, &mut App) + 'static>,
        cx: &mut App,
    ) -> Entity<Self> {
        cx.new(|cx| {
            // The database is set as a global after the window is initialized, so this will
            // always run after the window is initialized.
            let database_subscription =
                cx.observe_global::<Database>(|_, cx: &mut Context<AlbumLibrary>| {
                    let database = cx.global::<Database>();
                    let query = database.query_all_albums();

                    cx.spawn(
                        async move |albums_view: WeakEntity<Self>, cx: &mut AsyncApp| {
                            let albums_query = query.await;
                            albums_view
                                .update(cx, |albums_view, cx| {
                                    albums_view.albums_query =
                                        Some(Rc::new(RefCell::new(albums_query)));
                                    cx.notify();
                                })
                                .unwrap();
                        },
                    )
                    .detach();
                });

            AlbumLibrary {
                database_subscription,
                albums_query: None,
                on_album_click: Rc::new(on_album_click),
            }
        })
    }
}

impl Render for AlbumLibrary {
    fn render(&mut self, window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let theme = cx.global::<Theme>();

        let album_click_handler = self.on_album_click.clone();

        div()
            .bg(theme.background)
            .w_full()
            .h_full()
            .flex()
            .flex_col()
            .child(
                grandstand("albums-grandstand")
                    .text(tr!("LIBRARY_ALBUMS_TITLE", "Albums in Library"))
                    .pt(px(36.)),
            )
            .child(match self.albums_query.as_mut() {
                Some(albums_query) => {
                    let mut albums_query = albums_query.borrow_mut();
                    match albums_query.as_mut() {
                        Ok(albums_query) => albums_query
                            .iter(cx)
                            .enumerate()
                            .fold(
                                div()
                                    .id("album-grid")
                                    .grid()
                                    .grid_cols(3)
                                    .overflow_y_scroll(),
                                |david, (i, album)| {
                                    let album_clone = album.clone();
                                    let album_click_handler = album_click_handler.clone();
                                    david.child(
                                        div()
                                            .id(i)
                                            .child(album.clone().into_any_element())
                                            .on_click(move |_, window, cx| {
                                                album_click_handler.clone()(
                                                    &album_clone,
                                                    window,
                                                    cx,
                                                );
                                            }),
                                    )
                                },
                            )
                            .into_any_element(),
                        Err(_) => interstitial()
                            .w_full()
                            .h_full()
                            .icon("media-album-cover".into())
                            .title(tr!("LIBRARY_ALBUMS_ERROR", "Unable to load albums").into())
                            .message(tr!("LIBRARY_CORRUPT_ERROR_MESSAGE",).into())
                            .child(
                                button("tracks-corrupt-erase-button")
                                    .child(icon_text(
                                        "view-refresh".into(),
                                        tr!("LIBRARY_ERASE", "Erase Library").into(),
                                    ))
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
    }
}
