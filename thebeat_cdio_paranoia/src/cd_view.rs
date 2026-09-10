use std::cell::RefCell;
use std::rc::Rc;
use cntp_i18n::tr;
use contemporary::components::button::button;
use contemporary::components::grandstand::grandstand;
use contemporary::components::icon_text::icon_text;
use contemporary::components::layer::layer;
use contemporary::components::subtitle::subtitle;
use gpui::{
    AnyElement, App, AppContext, BorrowAppContext, Context, ElementId, Entity, InteractiveElement,
    IntoElement, ParentElement, Render, Rgba, SharedString, StatefulInteractiveElement, Styled,
    Window, div, px, uniform_list,
};
use lthebeat::other_sources::OtherSource;
use lthebeat::play_queue::PlayQueue;
use lthebeat::play_queue::media_item::MediaItem;
use lthebeat::ui::track_list_skeleton::track_list_skeleton;
use std::str::FromStr;
use udisks2::Client;
use url::Url;
use lthebeat::audio_library::database_query::DatabaseQuery;
use lthebeat::audio_library::track::Track;

pub struct CdSource {
    name: String,
    block_device: String,
    id: SharedString,
    num_tracks: u32,
    view: Entity<CdView>,
}

impl CdSource {
    pub fn new(dbus_client: &Client, block_device: &str, num_tracks: u32, drive_name: &str, cx: &mut App) -> Self {
        CdSource {
            name: drive_name.into(),
            block_device: block_device.to_string(),
            id: format!("cd-paranoia-{}", block_device).into(),
            num_tracks,
            view: cx.new(|cx| CdView::new(dbus_client, block_device, num_tracks, drive_name, cx)),
        }
    }
}

impl OtherSource for CdSource {
    fn render(&self) -> AnyElement {
        self.view.clone().into_any_element()
    }

    fn name(&self) -> &str {
        &self.name
    }

    fn id(&self) -> ElementId {
        self.id.clone().into()
    }
}

pub struct CdView {
    block_device: String,
    dbus_client: Client,
    num_tracks: u32,
    drive_name: String,
}

impl CdView {
    pub fn new(
        dbus_client: &Client,
        block_device: &str,
        num_tracks: u32,
        drive_name: &str,
        cx: &mut Context<Self>,
    ) -> Self {
        CdView {
            block_device: block_device.to_string(),
            dbus_client: dbus_client.clone(),
            num_tracks,
            drive_name: drive_name.to_string(),
        }
    }
}

impl CdView {
    fn play_all(&self, cx: &mut Context<Self>) {
        let play_queue = cx.global_mut::<PlayQueue>();
        play_queue.clear();

        self.enqueue_all(cx);
    }

    fn shuffle_all(&self, cx: &mut Context<Self>) {
        self.play_all(cx);

        cx.update_global::<PlayQueue, ()>(|play_queue, cx| {
            play_queue.shuffle(true, cx);
            play_queue.skip_next();
        });
    }

    fn enqueue_all(&self, cx: &mut Context<Self>) {
        let url_list: Vec<_> = (0..self.num_tracks).map(|track| {
            Url::from_str(&format!(
                "cd://{}?track={}",
                self.block_device,
                track + 1
            ))
                .unwrap()
        }).collect();

        cx.update_global::<PlayQueue, ()>(|play_queue, cx| {
            for url in url_list {
                let media_item = MediaItem::new(url, cx);
                play_queue.add_item(media_item, cx);
            }
        })
    }

}

impl Render for CdView {
    fn render(&mut self, window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let block_device = self.block_device.clone();

        div()
            .w_full()
            .h_full()
            .flex()
            .flex_col()
            .child(grandstand("cd-grandstand").text(self.drive_name.clone()).pt(px(36.)))
            .child(
                track_list_skeleton(
                    uniform_list(
                        "tracks-list",
                        self.num_tracks as usize,
                        move |range, _, cx| {
                            range
                                .map(|index| {
                                    let url = Url::from_str(&format!(
                                        "cd://{block_device}?track={}",
                                        index + 1
                                    ))
                                    .unwrap();

                                    div()
                                        .id(index)
                                        .child(tr!(
                                            "CD_TRACK_NUMBER",
                                            "Track {{number}}",
                                            number = (index + 1)
                                        ))
                                        .on_click(move |_, _, cx| {
                                            let item = MediaItem::new(url.clone(), cx);
                                            cx.update_global::<PlayQueue, ()>(|play_queue, cx| {
                                                play_queue.add_item(item, cx);
                                            })
                                        })
                                })
                                .collect()
                        },
                    )
                    .h_full()
                    .flex_grow(1.),
                )
                .track_count(self.num_tracks as usize)
                .side_list_area_child(
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
                                .on_click(cx.listener(move |this, _, _, cx| {
                                    this.play_all(cx);
                                })),
                        )
                        .child(
                            button("enqueue-all-button")
                                .flat()
                                .justify_start()
                                .child(icon_text(
                                    "view-media-playlist",
                                    tr!("TRACK_LISTING_ENQUEUE_ALL", "Enqueue All"),
                                ))
                                .on_click(cx.listener(move |this, _, _, cx| {
                                    this.enqueue_all(cx);
                                })),
                        )
                        .child(
                            button("shuffle-all-button")
                                .flat()
                                .justify_start()
                                .child(icon_text(
                                    "media-playlist-shuffle",
                                    tr!("TRACK_LISTING_SHUFFLE_ALL", "Shuffle All"),
                                ))
                                .on_click(cx.listener(move |this, _, _, cx| {
                                    this.shuffle_all(cx);
                                })),
                        ),
                ),
            )
    }
}
