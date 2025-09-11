use cntp_i18n::tr;
use contemporary::components::button::button;
use contemporary::components::grandstand::grandstand;
use contemporary::components::icon_text::icon_text;
use contemporary::components::interstitial::interstitial;
use contemporary::components::spinner::spinner;
use contemporary::styling::theme::Theme;
use gpui::private::anyhow;
use gpui::{
    App, AppContext, AsyncApp, BorrowAppContext, Bounds, Context, Element, ElementId, Entity,
    GlobalElementId, InspectorElementId, InteractiveElement, IntoElement, LayoutId, ParentElement,
    Pixels, Render, StatefulInteractiveElement, Style, Styled, Subscription, WeakEntity, Window,
    div, px,
};
use lthebeat::audio_library::artist::Artist;
use lthebeat::audio_library::database::Database;
use lthebeat::audio_library::database_query::DatabaseQuery;
use std::cell::RefCell;
use std::panic::Location;
use std::rc::Rc;

pub struct ArtistsLibrary {
    database_subscription: Subscription,
    artists_query: Option<Rc<RefCell<anyhow::Result<DatabaseQuery<Artist>>>>>,
    on_artists_click: Rc<Box<dyn Fn(&Entity<Artist>, &mut Window, &mut App) + 'static>>,
}

impl ArtistsLibrary {
    pub fn new(
        on_artists_click: Box<dyn Fn(&Entity<Artist>, &mut Window, &mut App) + 'static>,
        cx: &mut App,
    ) -> Entity<Self> {
        cx.new(|cx| {
            // The database is set as a global after the window is initialized, so this will
            // always run after the window is initialized.
            let database_subscription =
                cx.observe_global::<Database>(|_, cx: &mut Context<ArtistsLibrary>| {
                    let database = cx.global::<Database>();
                    let query = database.query_all_artists();

                    cx.spawn(
                        async move |aritsts_view: WeakEntity<Self>, cx: &mut AsyncApp| {
                            let aritsts_query = query.await;
                            aritsts_view
                                .update(cx, |aritsts_view, cx| {
                                    aritsts_view.artists_query =
                                        Some(Rc::new(RefCell::new(aritsts_query)));
                                    cx.notify();
                                })
                                .unwrap();
                        },
                    )
                    .detach();
                });

            ArtistsLibrary {
                database_subscription,
                artists_query: None,
                on_artists_click: Rc::new(on_artists_click),
            }
        })
    }
}

impl Render for ArtistsLibrary {
    fn render(&mut self, window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let theme = cx.global::<Theme>();

        let artists_click_handler = self.on_artists_click.clone();

        div()
            .bg(theme.background)
            .w_full()
            .h_full()
            .flex()
            .flex_col()
            .child(
                grandstand("aritsts-grandstand")
                    .text(tr!("LIBRARY_ARTISTS_TITLE", "Artists in Library"))
                    .pt(px(36.)),
            )
            .child(match self.artists_query.as_mut() {
                Some(aritsts_query) => {
                    let mut aritsts_query = aritsts_query.borrow_mut();
                    match aritsts_query.as_mut() {
                        Ok(aritsts_query) => aritsts_query
                            .iter(cx)
                            .enumerate()
                            .fold(
                                div()
                                    .id("artists-grid")
                                    .flex()
                                    .flex_wrap()
                                    .justify_around()
                                    .gap(px(24.))
                                    .overflow_y_scroll(),
                                |david, (i, artists)| {
                                    let artists_clone = artists.clone();
                                    let artists_click_handler = artists_click_handler.clone();
                                    david.child(
                                        div()
                                            .id(i)
                                            .child(artists.clone().into_any_element())
                                            .on_click(move |_, window, cx| {
                                                artists_click_handler.clone()(
                                                    &artists_clone,
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
                            .icon("media-artists-cover".into())
                            .title(tr!("LIBRARY_ARTISTS_ERROR", "Unable to load artists").into())
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
