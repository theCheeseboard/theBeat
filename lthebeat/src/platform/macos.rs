use crate::audio_processing::audio_controller::AudioController;
use crate::audio_processing::audio_metadata::AudioMetadata;
use crate::platform::PlatformHandler;
use crate::play_queue::PlayQueue;
use block2::RcBlock;
use gpui::{App, AppContext, AsyncApp, Entity};
use objc2::rc::Retained;
use objc2::runtime::ProtocolObject;
use objc2_foundation::{NSMutableDictionary, NSNumber, NSString};
use objc2_media_player::{
    MPChangePlaybackPositionCommandEvent, MPMediaItemPropertyAlbumTitle, MPMediaItemPropertyArtist,
    MPMediaItemPropertyMediaType, MPMediaItemPropertyPlaybackDuration, MPMediaItemPropertyTitle,
    MPNowPlayingInfoCenter, MPNowPlayingInfoMediaType, MPNowPlayingInfoPropertyElapsedPlaybackTime,
    MPNowPlayingInfoPropertyIsLiveStream, MPNowPlayingPlaybackState, MPRemoteCommandCenter,
    MPRemoteCommandEvent, MPRemoteCommandHandlerStatus,
};
use std::ptr::NonNull;
use std::time::Duration;

struct MacPlatform {
    current_metadata: Option<AudioMetadata>,
}

impl MacPlatform {
    pub fn propagate_changes_to_np_center(&self, cx: &mut App) {
        let audio_controller = cx.global::<AudioController>();
        let is_playing = audio_controller.is_playing();
        let current_time = audio_controller.current_time();

        unsafe {
            let dictionary: Retained<NSMutableDictionary<NSString>> =
                NSMutableDictionary::dictionary();
            if let Some(meta) = &self.current_metadata {
                dictionary.setObject_forKey(
                    &NSNumber::numberWithUnsignedInteger(MPNowPlayingInfoMediaType::Audio.0),
                    ProtocolObject::from_ref(MPMediaItemPropertyMediaType),
                );
                if let Some(title) = meta.title.as_ref() {
                    dictionary.setObject_forKey(
                        &NSString::from_str(title),
                        ProtocolObject::from_ref(MPMediaItemPropertyTitle),
                    );
                }
                if let Some(artist) = meta.artist.as_ref() {
                    dictionary.setObject_forKey(
                        &NSString::from_str(artist),
                        ProtocolObject::from_ref(MPMediaItemPropertyArtist),
                    );
                }
                if let Some(album) = meta.album.as_ref() {
                    dictionary.setObject_forKey(
                        &NSString::from_str(album),
                        ProtocolObject::from_ref(MPMediaItemPropertyAlbumTitle),
                    );
                }
                if let Some(duration) = meta.duration.as_ref() {
                    dictionary.setObject_forKey(
                        &NSNumber::numberWithDouble(duration.as_secs_f64()),
                        ProtocolObject::from_ref(MPMediaItemPropertyPlaybackDuration),
                    );
                    dictionary.setObject_forKey(
                        &NSNumber::numberWithBool(false),
                        ProtocolObject::from_ref(MPNowPlayingInfoPropertyIsLiveStream),
                    );
                } else {
                    dictionary.setObject_forKey(
                        &NSNumber::numberWithBool(true),
                        ProtocolObject::from_ref(MPNowPlayingInfoPropertyIsLiveStream),
                    );
                }
                if let Some(current_time) = current_time.as_ref() {
                    dictionary.setObject_forKey(
                        &NSNumber::numberWithDouble(current_time.as_secs_f64()),
                        ProtocolObject::from_ref(MPNowPlayingInfoPropertyElapsedPlaybackTime),
                    );
                }
            } else {
                dictionary.setObject_forKey(
                    &NSNumber::numberWithUnsignedInteger(MPNowPlayingInfoMediaType::None.0),
                    ProtocolObject::from_ref(MPMediaItemPropertyMediaType),
                );
            }

            let center = MPNowPlayingInfoCenter::defaultCenter();
            center.setNowPlayingInfo(Some(dictionary.as_ref()));

            center.setPlaybackState(if is_playing {
                MPNowPlayingPlaybackState::Playing
            } else {
                MPNowPlayingPlaybackState::Paused
            })
        }
    }
}

impl PlatformHandler for MacPlatform {
    fn new_metadata_available(&mut self, meta: AudioMetadata, cx: &mut App) {
        self.current_metadata = Some(meta);
        self.propagate_changes_to_np_center(cx);
    }

    fn play_state_changed(&mut self, cx: &mut App) {
        self.propagate_changes_to_np_center(cx);
    }
}

enum MediaPlayerEvent {
    Play,
    Pause,
    PlayPause,
    SkipBack,
    SkipForward,
    Seek(Duration),
}

pub fn create_platform(cx: &mut App) -> Entity<Box<dyn PlatformHandler>> {
    cx.new(|cx| -> Box<dyn PlatformHandler> {
        let platform = MacPlatform {
            current_metadata: None,
        };

        let (tx_event, rx_event) = async_channel::bounded(3);

        unsafe {
            let command_center = MPRemoteCommandCenter::sharedCommandCenter();

            let tx_play = tx_event.clone();
            command_center.playCommand().setEnabled(true);
            command_center
                .playCommand()
                .addTargetWithHandler(&RcBlock::new(move |_| {
                    smol::block_on(tx_play.send(MediaPlayerEvent::Play)).unwrap();
                    MPRemoteCommandHandlerStatus::Success
                }));

            let tx_pause = tx_event.clone();
            command_center.pauseCommand().setEnabled(true);
            command_center
                .pauseCommand()
                .addTargetWithHandler(&RcBlock::new(move |_| {
                    smol::block_on(tx_pause.send(MediaPlayerEvent::Pause)).unwrap();
                    MPRemoteCommandHandlerStatus::Success
                }));

            let tx_play_pause = tx_event.clone();
            command_center.togglePlayPauseCommand().setEnabled(true);
            command_center
                .togglePlayPauseCommand()
                .addTargetWithHandler(&RcBlock::new(move |_| {
                    smol::block_on(tx_play_pause.send(MediaPlayerEvent::PlayPause)).unwrap();
                    MPRemoteCommandHandlerStatus::Success
                }));

            let tx_skip_back = tx_event.clone();
            command_center.previousTrackCommand().setEnabled(true);
            command_center
                .previousTrackCommand()
                .addTargetWithHandler(&RcBlock::new(move |_| {
                    smol::block_on(tx_skip_back.send(MediaPlayerEvent::SkipBack)).unwrap();
                    MPRemoteCommandHandlerStatus::Success
                }));

            let tx_skip_forward = tx_event.clone();
            command_center.nextTrackCommand().setEnabled(true);
            command_center
                .nextTrackCommand()
                .addTargetWithHandler(&RcBlock::new(move |_| {
                    smol::block_on(tx_skip_forward.send(MediaPlayerEvent::SkipForward)).unwrap();
                    MPRemoteCommandHandlerStatus::Success
                }));

            let tx_seek = tx_event.clone();
            command_center
                .changePlaybackPositionCommand()
                .setEnabled(true);
            command_center
                .changePlaybackPositionCommand()
                .addTargetWithHandler(&RcBlock::new(
                    move |event: NonNull<MPRemoteCommandEvent>| {
                        let change_playback_position_event = event
                            .as_ref()
                            .downcast_ref::<MPChangePlaybackPositionCommandEvent>()
                            .unwrap();
                        let position =
                            Duration::from_secs_f64(change_playback_position_event.positionTime());
                        smol::block_on(tx_seek.send(MediaPlayerEvent::Seek(position))).unwrap();
                        MPRemoteCommandHandlerStatus::Success
                    },
                ));
        };

        cx.spawn(async move |_, cx: &mut AsyncApp| {
            loop {
                match rx_event.recv().await {
                    Ok(event) => match event {
                        MediaPlayerEvent::Play => cx
                            .update_global::<AudioController, ()>(|audio_controller, cx| {
                                audio_controller.play();
                            })
                            .unwrap(),
                        MediaPlayerEvent::Pause => cx
                            .update_global::<AudioController, ()>(|audio_controller, cx| {
                                audio_controller.pause();
                            })
                            .unwrap(),
                        MediaPlayerEvent::PlayPause => cx
                            .update_global::<AudioController, ()>(|audio_controller, cx| {
                                audio_controller.play_pause();
                            })
                            .unwrap(),
                        MediaPlayerEvent::SkipBack => cx
                            .update_global::<PlayQueue, ()>(|play_queue, cx| {
                                play_queue.skip_previous();
                            })
                            .unwrap(),
                        MediaPlayerEvent::SkipForward => cx
                            .update_global::<PlayQueue, ()>(|play_queue, cx| {
                                play_queue.skip_next();
                            })
                            .unwrap(),
                        MediaPlayerEvent::Seek(position) => cx
                            .update_global::<PlayQueue, ()>(|play_queue, cx| {
                                play_queue.seek_to_position(position)
                            })
                            .unwrap(),
                    },
                    Err(_) => {
                        return;
                    }
                }
            }
        })
        .detach();

        cx.observe_global::<AudioController>(|platform, cx| {
            platform.play_state_changed(cx);
        })
        .detach();

        Box::new(platform)
    })
}
