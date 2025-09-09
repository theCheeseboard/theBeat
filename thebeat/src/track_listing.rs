use cntp_i18n::{tr, trn};
use contemporary::components::button::button;
use contemporary::components::icon::icon;
use contemporary::components::icon_text::icon_text;
use contemporary::components::layer::layer;
use contemporary::components::subtitle::subtitle;
use gpui::private::anyhow;
use gpui::{
    App, Entity, InteractiveElement, IntoElement, ParentElement, RenderOnce, Styled, Window, div,
    px, uniform_list,
};
use lthebeat::audio_library::database_query::DatabaseQuery;
use lthebeat::audio_library::track::Track;
use lthebeat::play_queue::PlayQueue;
use lthebeat::play_queue::media_item::MediaItem;
use std::cell::RefCell;
use std::rc::Rc;

#[derive(IntoElement)]
pub struct TrackListing {
    database_query: Rc<RefCell<anyhow::Result<DatabaseQuery<Track>>>>,
}

pub fn track_listing(
    database_query: Rc<RefCell<anyhow::Result<DatabaseQuery<Track>>>>,
) -> TrackListing {
    TrackListing { database_query }
}

impl RenderOnce for TrackListing {
    fn render(self, _: &mut Window, _: &mut App) -> impl IntoElement {
        let database_query_clone = self.database_query.clone();
        let database_query_clone_2 = self.database_query.clone();
        let database_query_clone_3 = self.database_query.clone();
        let track_count = self.database_query.borrow().as_ref().unwrap().count();
        div()
            .id("track-listing")
            .flex()
            .child(
                div()
                    .p(px(8.))
                    .gap(px(8.))
                    .flex()
                    .flex_col()
                    .child(
                        layer()
                            .p(px(8.))
                            .gap(px(8.))
                            .flex()
                            .flex_col()
                            .child(subtitle(
                                tr!("TRACK_LISTING_ACTIONS", "Actions").to_uppercase(),
                            ))
                            .child(
                                button("play-all-button")
                                    .flat()
                                    .justify_start()
                                    .child(icon_text(
                                        "media-playback-start".into(),
                                        tr!("TRACK_LISTING_PLAY_ALL", "Play All").into(),
                                    ))
                                    .on_click(move |_, _, cx| {
                                        play_all(database_query_clone_2.clone(), cx);
                                    }),
                            )
                            .child(
                                button("enqueue-all-button")
                                    .flat()
                                    .justify_start()
                                    .child(icon_text(
                                        "view-media-playlist".into(),
                                        tr!("TRACK_LISTING_ENQUEUE_ALL", "Enqueue All").into(),
                                    ))
                                    .on_click(move |_, _, cx| {
                                        enqueue_all(database_query_clone_3.clone(), cx);
                                    }),
                            )
                            .child(button("shuffle-all-button").flat().justify_start().child(
                                icon_text(
                                    "media-playlist-shuffle".into(),
                                    tr!("TRACK_LISTING_SHUFFLE_ALL", "Shuffle All").into(),
                                ),
                            ))
                            .child(
                                button("burn-button")
                                    .flat()
                                    .justify_start()
                                    .child(icon_text(
                                        "tools-media-optical-burn".into(),
                                        tr!("TRACK_LISTING_BURN", "Burn").into(),
                                    )),
                            ),
                    )
                    .child(div().flex_grow())
                    .child(trn!(
                        "TRACK_LISTING_TRACK_COUNT",
                        "{{count}} track",
                        "{{count}} tracks",
                        count = track_count as isize
                    )),
            )
            .child(
                uniform_list("tracks-list", track_count, move |range, _, cx| {
                    range
                        .map(|index| {
                            database_query_clone
                                .borrow_mut()
                                .as_mut()
                                .unwrap()
                                .get(index, cx)
                        })
                        .collect()
                })
                .h_full()
                .flex_grow(),
            )
            .h_full()
            .w_full()
    }
}

fn play_all(database_query: Rc<RefCell<anyhow::Result<DatabaseQuery<Track>>>>, cx: &mut App) {
    let play_queue = cx.global_mut::<PlayQueue>();
    play_queue.clear();

    enqueue_all(database_query, cx);
}

fn enqueue_all(database_query: Rc<RefCell<anyhow::Result<DatabaseQuery<Track>>>>, cx: &mut App) {
    let track_list: Vec<_> = database_query
        .borrow_mut()
        .as_mut()
        .unwrap()
        .iter(cx)
        .collect();

    for track in track_list {
        let Track::Ok { url, .. } = track.read(cx) else {
            continue;
        };
        let item = MediaItem::new(url.clone(), cx);
        let play_queue = cx.global_mut::<PlayQueue>();
        play_queue.add_item(item);
    }
}
