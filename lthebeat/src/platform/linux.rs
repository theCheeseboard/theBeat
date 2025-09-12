use crate::audio_processing::audio_controller::AudioController;
use crate::audio_processing::audio_metadata::{Art, AudioMetadata};
use crate::platform::PlatformHandler;
use crate::play_queue::PlayQueue;
use contemporary::application::{Details, GeneratableDetails};
use gpui::{App, AppContext, AsyncApp, Entity, WeakEntity};
use mpris_server::zbus::zvariant::ObjectPath;
use mpris_server::{Metadata, PlaybackStatus, Player, Time, Uri};
use std::any::Any;
use std::io::Write;
use std::rc::Rc;
use std::sync::Arc;
use std::time::Duration;
use tempfile::NamedTempFile;
use tracing::error;
use url::Url;

struct LinuxPlatform {
    mpris_player: Option<Rc<Player>>,
    current_metadata: Option<AudioMetadata>,
    current_art: Option<Arc<Art>>,
    art_temp_file: Option<NamedTempFile>,
}

impl PlatformHandler for LinuxPlatform {
    fn new_metadata_available(&mut self, meta: AudioMetadata, cx: &mut App) {
        if self.current_art.as_ref().map(|art| Arc::as_ptr(art))
            != meta.album_cover.as_ref().map(|art| Arc::as_ptr(art))
        {
            self.art_temp_file = None;

            if let Some(new_art) = meta.album_cover.as_ref() {
                let mut temp_file = NamedTempFile::new().unwrap();
                temp_file.write_all(new_art.backing_store.as_ref()).unwrap();
                self.art_temp_file = Some(temp_file);
            }
            self.current_art = meta.album_cover.clone();
        }

        self.current_metadata = Some(meta);

        self.update_mpris(cx);
    }

    fn play_state_changed(&mut self, is_playing: bool, cx: &mut App) {
        let Some(player) = &self.mpris_player else {
            return;
        };
        let player = player.clone();

        cx.spawn(async move |_| {
            player
                .set_playback_status(if is_playing {
                    PlaybackStatus::Playing
                } else {
                    PlaybackStatus::Paused
                })
                .await
                .unwrap();
        })
        .detach();
    }

    fn seek_performed(&mut self, current_time: Duration, cx: &mut App) {
        let Some(player) = &self.mpris_player else {
            return;
        };
        let player = player.clone();

        cx.spawn(async move |_: &mut AsyncApp| {
            player
                .seeked(Time::from_millis(current_time.as_millis() as i64))
                .await
                .unwrap();
        })
        .detach();
    }

    fn as_any(&self) -> &dyn Any {
        self
    }
    fn as_any_mut(&mut self) -> &mut dyn Any {
        self
    }
}

impl LinuxPlatform {
    pub fn update_mpris(&self, cx: &mut App) {
        let Some(player) = &self.mpris_player else {
            return;
        };
        let player = player.clone();

        let audio_controller = cx.global::<AudioController>();
        let is_playing = audio_controller.is_playing();
        let current_time = audio_controller.current_time();

        let mut mpris_meta = Metadata::default();
        if let Some(meta) = &self.current_metadata {
            mpris_meta.set_title(meta.title.clone());
            mpris_meta.set_artist(meta.artist.clone().map(|artist| vec![artist]));
            mpris_meta.set_album(meta.album.clone());
            mpris_meta.set_track_number(meta.track_number.map(|track_number| track_number as i32));
            mpris_meta.set_length(
                meta.duration
                    .map(|duration| Time::from_millis(duration.as_millis() as i64)),
            );
            mpris_meta.set_trackid(Some(ObjectPath::from_static_str_unchecked(
                "/com/vicr123/thebeat/currentmedia",
            )));
            mpris_meta.set_art_url(self.art_temp_file.as_ref().map(|art_temp_file| {
                Url::from_file_path(art_temp_file.path())
                    .expect("URL should be a valid file path")
                    .to_string()
            }));
        }

        if let Some(current_time) = current_time {
            player.set_position(Time::from_millis(current_time.as_millis() as i64));
        }

        cx.spawn(async move |_: &mut AsyncApp| {
            player
                .set_can_seek(mpris_meta.length().is_some())
                .await
                .unwrap();
            player.set_metadata(mpris_meta).await.unwrap();
            player
                .set_playback_status(if is_playing {
                    PlaybackStatus::Playing
                } else {
                    PlaybackStatus::Paused
                })
                .await
                .unwrap();
        })
        .detach();
    }

    fn update_art(&self, cx: &mut App) {}
}

pub fn create_platform(cx: &mut App) -> Entity<Box<dyn PlatformHandler>> {
    cx.new(|cx| -> Box<dyn PlatformHandler> {
        let details = cx.global::<Details>();
        let desktop_entry = details.generatable.desktop_entry;
        let application_name = details.generatable.application_name.default_value();

        cx.spawn(
            async move |weak_this: WeakEntity<Box<dyn PlatformHandler>>, cx: &mut AsyncApp| {
                let player = Player::builder("thebeat")
                    .can_play(true)
                    .can_pause(true)
                    .can_go_next(true)
                    .can_go_previous(true)
                    .desktop_entry(desktop_entry)
                    .identity(application_name)
                    .build()
                    .await;

                match player {
                    Ok(player) => {
                        let play_async_cx = cx.clone();
                        player.connect_play(move |_| {
                            play_async_cx
                                .update_global::<AudioController, ()>(
                                    move |audio_controller, cx| {
                                        audio_controller.play(cx);
                                    },
                                )
                                .unwrap();
                        });

                        let pause_async_cx = cx.clone();
                        player.connect_pause(move |_| {
                            pause_async_cx
                                .update_global::<AudioController, ()>(
                                    move |audio_controller, cx| {
                                        audio_controller.pause(cx);
                                    },
                                )
                                .unwrap();
                        });

                        let play_pause_async_cx = cx.clone();
                        player.connect_play_pause(move |_| {
                            play_pause_async_cx
                                .update_global::<AudioController, ()>(
                                    move |audio_controller, cx| {
                                        audio_controller.play_pause(cx);
                                    },
                                )
                                .unwrap();
                        });

                        let skip_back_async_cx = cx.clone();
                        player.connect_next(move |_| {
                            skip_back_async_cx
                                .update_global::<PlayQueue, ()>(move |play_queue, _| {
                                    play_queue.skip_previous();
                                })
                                .unwrap();
                        });

                        let skip_next_async_cx = cx.clone();
                        player.connect_next(move |_| {
                            skip_next_async_cx
                                .update_global::<PlayQueue, ()>(move |play_queue, _| {
                                    play_queue.skip_next();
                                })
                                .unwrap();
                        });

                        let seek_async_cx = cx.clone();
                        player.connect_set_position(move |_, _, time| {
                            seek_async_cx
                                .update_global::<PlayQueue, ()>(move |play_queue, cx| {
                                    play_queue.seek_to_position(
                                        Duration::from_millis(time.as_millis() as u64),
                                        cx,
                                    );
                                })
                                .unwrap();
                        });

                        let player_rc = Rc::new(player);
                        let player_rc_clone = player_rc.clone();
                        weak_this
                            .update(cx, |this, cx| {
                                let linux_platform = this
                                    .as_any_mut()
                                    .downcast_mut::<LinuxPlatform>()
                                    .expect("PlatformHandler should be LinuxPlatform");
                                linux_platform.mpris_player = Some(player_rc_clone);
                                linux_platform.update_mpris(cx);
                            })
                            .unwrap();

                        // Start the player event loop
                        player_rc.run().await;
                    }
                    Err(e) => {
                        error!("Unable to connect to MPRIS: {}", e);
                    }
                }
            },
        )
        .detach();

        Box::new(LinuxPlatform {
            mpris_player: None,
            current_metadata: None,
            art_temp_file: None,
            current_art: None,
        })
    })
}
