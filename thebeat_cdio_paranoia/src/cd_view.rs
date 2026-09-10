use crate::cdio_paranoia_engine::cdio_manager::CdioManager;
use crate::track_url;
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
use lthebeat::audio_library::database_query::DatabaseQuery;
use lthebeat::audio_library::track::Track;
use lthebeat::audio_processing::audio_metadata::AudioMetadata;
use lthebeat::metadata_registry::MetadataRegistry;
use lthebeat::other_sources::OtherSource;
use lthebeat::play_queue::PlayQueue;
use lthebeat::play_queue::media_item::MediaItem;
use lthebeat::ui::track_list_skeleton::track_list_skeleton;
use std::cell::RefCell;
use std::rc::Rc;
use std::str::FromStr;
use udisks2::Client;
use url::Url;

pub struct CdSource {
    name: String,
    id: SharedString,
    view: Entity<CdView>,
}

impl CdSource {
    pub fn new(
        dbus_client: &Client,
        block_device: &str,
        num_tracks: u32,
        drive_name: &str,
        cx: &mut App,
    ) -> Self {
        let cdio_manager = cx.global::<CdioManager>();
        if let Ok(cd_device) = cdio_manager.get_cd(block_device) {
            cx.update_global::<MetadataRegistry, _>(|metadata_registry, cx| {
                for i in cd_device.first_track()..=cd_device.last_track() {
                    let url = track_url(block_device, i);
                    metadata_registry.insert_metadata(
                        url.clone(),
                        AudioMetadata {
                            url: Some(url),
                            associated_item: None,
                            title: Some(tr!("CD_TRACK_NUMBER", number = i).into()),
                            album: Some(drive_name.into()),
                            track_number: Some(i as u32),
                            total_track_number: Some(num_tracks + 1),
                            ..Default::default()
                        },
                    )
                }
            });

            CdSource {
                name: drive_name.into(),
                id: format!("cd-paranoia-{}", block_device).into(),
                view: cx.new(|cx| {
                    CdView::new(
                        dbus_client,
                        block_device,
                        cd_device.first_track() as u32,
                        cd_device.last_track() as u32,
                        drive_name,
                        cx,
                    )
                }),
            }
        } else {
            cx.update_global::<MetadataRegistry, _>(|metadata_registry, cx| {
                for i in 0..num_tracks {
                    let url = track_url(block_device, i + 1);
                    metadata_registry.insert_metadata(
                        url.clone(),
                        AudioMetadata {
                            url: Some(url),
                            associated_item: None,
                            title: Some(tr!("CD_TRACK_NUMBER", number = (i + 1)).into()),
                            album: Some(drive_name.into()),
                            track_number: Some(i + 1),
                            total_track_number: Some(num_tracks + 1),
                            ..Default::default()
                        },
                    )
                }
            });

            CdSource {
                name: drive_name.into(),
                id: format!("cd-paranoia-{}", block_device).into(),
                view: cx.new(|cx| {
                    CdView::new(dbus_client, block_device, 1, 1 + num_tracks, drive_name, cx)
                }),
            }
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
    first_track: u32,
    last_track: u32,
    drive_name: String,
}

impl CdView {
    pub fn new(
        dbus_client: &Client,
        block_device: &str,
        first_track: u32,
        last_track: u32,
        drive_name: &str,
        cx: &mut Context<Self>,
    ) -> Self {
        CdView {
            block_device: block_device.to_string(),
            dbus_client: dbus_client.clone(),
            first_track,
            last_track,
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
        let url_list: Vec<_> = (self.first_track..=self.last_track)
            .map(|track| track_url(&self.block_device, track))
            .collect();

        cx.update_global::<PlayQueue, ()>(|play_queue, cx| {
            for url in url_list {
                let media_item = cx.new(|cx| MediaItem::new(url, cx));
                play_queue.add_item(media_item, cx);
            }
        })
    }
}

impl Render for CdView {
    fn render(&mut self, window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let block_device = self.block_device.clone();
        let num_tracks = (self.last_track - self.first_track + 1) as usize;

        div()
            .w_full()
            .h_full()
            .flex()
            .flex_col()
            .child(
                grandstand("cd-grandstand")
                    .text(self.drive_name.clone())
                    .pt(px(36.)),
            )
            .child(
                track_list_skeleton(
                    uniform_list("tracks-list", num_tracks, move |range, _, cx| {
                        range
                            .map(|index| {
                                let url = track_url(&block_device, index + 1);

                                div()
                                    .id(index)
                                    .child(tr!(
                                        "CD_TRACK_NUMBER",
                                        "Track {{number}}",
                                        number = (index + 1)
                                    ))
                                    .on_click(move |_, _, cx| {
                                        let item = cx.new(|cx| MediaItem::new(url.clone(), cx));
                                        cx.update_global::<PlayQueue, ()>(|play_queue, cx| {
                                            play_queue.add_item(item, cx);
                                        })
                                    })
                            })
                            .collect()
                    })
                    .h_full()
                    .flex_grow(1.),
                )
                .track_count(num_tracks)
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
