use cntp_i18n::tr;
use contemporary::components::grandstand::grandstand;
use contemporary::components::spinner::spinner;
use contemporary::styling::theme::Theme;
use gpui::private::anyhow;
use gpui::{
    App, AppContext, AsyncApp, Context, Entity, InteractiveElement, IntoElement, ParentElement,
    Render, StatefulInteractiveElement, Styled, Subscription, WeakEntity, Window, div, px,
};
use lthebeat::audio_library::album::Album;
use lthebeat::audio_library::database::Database;
use lthebeat::audio_library::database_query::DatabaseQuery;
use std::cell::RefCell;
use std::ops::Deref;
use std::rc::Rc;

pub struct AlbumsView {
    database_subscription: Subscription,
    albums_query: Option<Rc<RefCell<anyhow::Result<DatabaseQuery<Album>>>>>,
}

impl AlbumsView {
    pub fn new(cx: &mut App) -> Entity<Self> {
        cx.new(|cx| {
            // The database is set as a global after the window is initialized, so this will
            // always run after the window is initialized.
            let database_subscription =
                cx.observe_global::<Database>(|_, cx: &mut Context<AlbumsView>| {
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

            AlbumsView {
                database_subscription,
                albums_query: None,
            }
        })
    }
}

impl Render for AlbumsView {
    fn render(&mut self, window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let theme = cx.global::<Theme>();

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
                            .fold(
                                div()
                                    .id("album-grid")
                                    .grid()
                                    .grid_cols(3)
                                    .overflow_y_scroll(),
                                |div, album| div.child(album.clone().into_any_element()),
                            )
                            .into_any_element(),
                        Err(_) => div()
                            .child(tr!("LIBRARY_ALBUMS_ERROR", "Error loading albums"))
                            .into_any_element(),
                    }
                }
                _ => div().child(spinner()).into_any_element(),
            })
    }
}
