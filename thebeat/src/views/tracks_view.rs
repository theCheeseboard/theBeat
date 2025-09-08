use crate::track_listing::track_listing;
use cntp_i18n::tr;
use contemporary::components::button::button;
use contemporary::components::grandstand::grandstand;
use contemporary::components::icon_text::icon_text;
use contemporary::components::interstitial::interstitial;
use contemporary::components::spinner::spinner;
use contemporary::styling::theme::Theme;
use gpui::private::anyhow;
use gpui::{
    App, AppContext, AsyncApp, BorrowAppContext, Context, Entity, IntoElement, ParentElement,
    Render, Styled, Subscription, WeakEntity, Window, div, px, uniform_list,
};
use lthebeat::audio_library::database::Database;
use lthebeat::audio_library::database_query::DatabaseQuery;
use lthebeat::audio_library::track::Track;
use std::cell::RefCell;
use std::ops::Deref;
use std::rc::Rc;

pub struct TracksView {
    database_subscription: Subscription,
    tracks_query: Option<Rc<RefCell<anyhow::Result<DatabaseQuery<Track>>>>>,
}

impl TracksView {
    pub fn new(cx: &mut App) -> Entity<Self> {
        cx.new(|cx| {
            // The database is set as a global after the window is initialized, so this will
            // always run after the window is initialized.
            let database_subscription =
                cx.observe_global::<Database>(|_, cx: &mut Context<TracksView>| {
                    let database = cx.global::<Database>();
                    let query = database.query_all_tracks();

                    cx.spawn(
                        async move |tracks_view: WeakEntity<Self>, cx: &mut AsyncApp| {
                            let tracks_query = query.await;
                            tracks_view
                                .update(cx, |tracks_view, cx| {
                                    tracks_view.tracks_query =
                                        Some(Rc::new(RefCell::new(tracks_query)));
                                    cx.notify();
                                })
                                .unwrap();
                        },
                    )
                    .detach();
                });

            TracksView {
                database_subscription,
                tracks_query: None,
            }
        })
    }
}

impl Render for TracksView {
    fn render(&mut self, window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let theme = cx.global::<Theme>();

        div()
            .bg(theme.background)
            .w_full()
            .h_full()
            .flex()
            .flex_col()
            .child(
                grandstand("tracks-grandstand")
                    .text(tr!("LIBRARY_TRACKS_TITLE", "Tracks in Library"))
                    .pt(px(36.)),
            )
            .child(match self.tracks_query.as_mut() {
                Some(tracks_query) => match tracks_query.borrow().deref() {
                    Ok(_) => div()
                        .flex_grow()
                        .w_full()
                        .child(track_listing(tracks_query.clone()))
                        .into_any_element(),
                    Err(_) => interstitial()
                        .w_full()
                        .h_full()
                        .icon("view-media-track".into())
                        .title(tr!("LIBRARY_TRACKS_ERROR", "Unable to load tracks").into())
                        .message(
                            tr!(
                                "LIBRARY_CORRUPT_ERROR_MESSAGE",
                                "Your library may be corrupt. Try erasing your library."
                            )
                            .into(),
                        )
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
                },
                _ => div().child(spinner()).into_any_element(),
            })
    }
}
