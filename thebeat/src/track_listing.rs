use cntp_i18n::{tr, trn};
use contemporary::components::button::button;
use contemporary::components::icon_text::icon_text;
use contemporary::components::layer::layer;
use contemporary::components::subtitle::subtitle;
use gpui::private::anyhow;
use gpui::{
    App, BorrowAppContext, InteractiveElement, IntoElement, ParentElement, RenderOnce, Styled,
    Window, div, px, uniform_list,
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
        let database_query_clone_4 = self.database_query.clone();
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
                                        "media-playback-start",
                                        tr!("TRACK_LISTING_PLAY_ALL", "Play All"),
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
                                        "view-media-playlist",
                                        tr!("TRACK_LISTING_ENQUEUE_ALL", "Enqueue All"),
                                    ))
                                    .on_click(move |_, _, cx| {
                                        enqueue_all(database_query_clone_3.clone(), cx);
                                    }),
                            )
                            .child(
                                button("shuffle-all-button")
                                    .flat()
                                    .justify_start()
                                    .child(icon_text(
                                        "media-playlist-shuffle",
                                        tr!("TRACK_LISTING_SHUFFLE_ALL", "Shuffle All"),
                                    ))
                                    .on_click(move |_, _, cx| {
                                        shuffle_all(database_query_clone_4.clone(), cx);
                                    }),
                            )
                            .child(
                                button("burn-button")
                                    .flat()
                                    .justify_start()
                                    .child(icon_text(
                                        "tools-media-optical-burn",
                                        tr!("TRACK_LISTING_BURN", "Burn"),
                                    )),
                            ),
                    )
                    .child(div().flex_grow(1.))
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
                .flex_grow(1.),
            )
            .h_full()
            .w_full()
    }
}

fn shuffle_all(database_query: Rc<RefCell<anyhow::Result<DatabaseQuery<Track>>>>, cx: &mut App) {
    play_all(database_query, cx);

    cx.update_global::<PlayQueue, ()>(|play_queue, cx| {
        play_queue.shuffle(true, cx);
        play_queue.skip_next();
    });
}

fn play_all(database_query: Rc<RefCell<anyhow::Result<DatabaseQuery<Track>>>>, cx: &mut App) {
    let play_queue = cx.global_mut::<PlayQueue>();
    play_queue.clear();

    enqueue_all(database_query, cx);
}

fn enqueue_all(database_query: Rc<RefCell<anyhow::Result<DatabaseQuery<Track>>>>, cx: &mut App) {
    let mut database_query_borrow = database_query.borrow_mut();
    let database_query = database_query_borrow.as_mut().unwrap();
    smol::block_on(database_query.populate_all(cx));

    let track_list: Vec<_> = database_query.iter(cx).collect();

    let url_list: Vec<_> = track_list
        .iter()
        .map(|track| track.read(cx))
        .filter_map(|track| match track {
            Track::Ok { url, .. } => Some(url.clone()),
            _ => None,
        })
        .collect();

    cx.update_global::<PlayQueue, ()>(|play_queue, cx| {
        for url in url_list {
            let media_item = MediaItem::new(url, cx);
            play_queue.add_item(media_item, cx);
        }
    })
}
