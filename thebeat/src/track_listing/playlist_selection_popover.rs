use std::cell::RefCell;
use std::rc::Rc;
use cntp_i18n::tr;
use contemporary::components::constrainer::constrainer;
use contemporary::components::grandstand::grandstand;
use contemporary::components::layer::layer;
use contemporary::components::popover::popover;
use gpui::{div, App, AsyncApp, Entity, IntoElement, RenderOnce, WeakEntity, Window, px, uniform_list, ListSizingBehavior, StatefulInteractiveElement, ParentElement, InteractiveElement, Styled};
use gpui::prelude::FluentBuilder;
use lthebeat::audio_library::database::Database;
use lthebeat::audio_library::database_query::DatabaseQuery;
use lthebeat::audio_library::playlist::Playlist;

type PlaylistSelectedEvent = Rc<dyn Fn(usize, &mut Window, &mut App)>;

#[derive(IntoElement)]
pub struct PlaylistSelectionPopover {
    pub visible: Entity<bool>,
    pub on_select: PlaylistSelectedEvent,
}

impl RenderOnce for PlaylistSelectionPopover {
    fn render(self, window: &mut Window, cx: &mut App) -> impl IntoElement {
        let popover_open_clone = self.visible.clone();

        let playlists_query =
            window.use_state::<Option<RefCell<DatabaseQuery<Playlist>>>>(cx, |_, cx| {
                let database = cx.global::<Database>();
                let query = database.query_all_playlists();

                cx.spawn(
                    async move |playlists_view: WeakEntity<
                        Option<RefCell<DatabaseQuery<Playlist>>>,
                    >,
                                cx: &mut AsyncApp| {
                        let playlist_query = query.await;
                        if let Some(playlists_view) = playlists_view.upgrade() {
                            playlists_view.write(
                                cx,
                                playlist_query
                                    .ok()
                                    .map(|playlist_query| RefCell::new(playlist_query)),
                            )
                        }
                    },
                )
                    .detach();

                None
            });

        popover("add-to-playlist-popover")
            .visible(*self.visible.read(cx))
            .size_neg(100.)
            .anchor_bottom()
            .content(
                div()
                    .flex()
                    .flex_col()
                    .gap(px(9.))
                    .child(
                        grandstand("add-to-playlist-grandstand")
                            .text(tr!("ADD_TO_PLAYLIST_TITLE", "Add to playlist"))
                            .on_back_click(move |_, _, cx| {
                                popover_open_clone.write(cx, false);
                            }),
                    )
                    .child(
                        constrainer("add-to-playlist-constrainer").child(
                            layer()
                                .flex()
                                .flex_col()
                                .p(px(8.))
                                .w_full()
                                .child(tr!("ADD_TO_PLAYLIST_PROMPT", "Which playlist?"))
                                .when_some(playlists_query.read(cx).as_ref(), |david, query| {
                                    let playlists_query = playlists_query.clone();
                                    david.child(
                                        uniform_list("playlist_list", query.borrow().count(), {
                                            let on_select = self.on_select.clone();
                                            move |range, _, cx| {
                                                playlists_query.update(cx, {
                                                    let on_select = on_select.clone();
                                                    move |playlists_query, cx| {
                                                        let playlists_query =
                                                            playlists_query.as_ref().unwrap();
                                                        range
                                                            .map(move |index| match playlists_query
                                                                .borrow_mut()
                                                                .get(index, cx)
                                                                .read(cx)
                                                            {
                                                                Playlist::Ok {
                                                                    name, id, ..
                                                                } => div().child(
                                                                    div()
                                                                        .id(*id)
                                                                        .child(name.clone())
                                                                        .on_click({
                                                                            let on_select =
                                                                                on_select.clone();
                                                                            let id = *id;
                                                                            move |_, window, cx| {
                                                                                on_select(
                                                                                    id, window, cx,
                                                                                );
                                                                            }
                                                                        }),
                                                                ),
                                                                _ => div(),
                                                            })
                                                            .collect()
                                                    }
                                                })
                                            }
                                        })
                                            .with_sizing_behavior(ListSizingBehavior::Infer),
                                    )
                                }),
                        ),
                    )
                    .into_any_element(),
            )
    }
}
