#include "nowplayingintegration.h"

#include <statemanager.h>
#include <playlist.h>
#include <QDateTime>
#include <QVariant>
#include <QUrl>
#include <QLocale>
#import <MediaPlayer/MediaPlayer.h>

struct NowPlayingIntegrationPrivate {
    MediaItem* currentItem = nullptr;
};

NowPlayingIntegration::NowPlayingIntegration(QObject* parent) : QObject(parent) {
    d = new NowPlayingIntegrationPrivate();
    Playlist* playlist = StateManager::instance()->playlist();

    connect(playlist, &Playlist::stateChanged, this, &NowPlayingIntegration::updateMetadata);
    connect(playlist, &Playlist::repeatOneChanged, this, &NowPlayingIntegration::updateMetadata);
    connect(playlist, &Playlist::shuffleChanged, this, &NowPlayingIntegration::updateMetadata);
    connect(playlist, &Playlist::volumeChanged, this, &NowPlayingIntegration::updateMetadata);

    connect(playlist, &Playlist::currentItemChanged, this, &NowPlayingIntegration::updateCurrentItem);
    updateCurrentItem();

    MPRemoteCommandCenter* commandCenter = [MPRemoteCommandCenter sharedCommandCenter];
    [[commandCenter playCommand] addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent * event) {
                                    playlist->play();
                                    return MPRemoteCommandHandlerStatusSuccess;
                                }];
    [[commandCenter pauseCommand] addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent * event) {
                                     playlist->pause();
                                     return MPRemoteCommandHandlerStatusSuccess;
                                 }];
    [[commandCenter togglePlayPauseCommand] addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent * event) {
                                               playlist->playPause();
                                               return MPRemoteCommandHandlerStatusSuccess;
                                           }];
    [[commandCenter stopCommand] setEnabled:NO];
    [[commandCenter nextTrackCommand] addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent * event) {
                                         playlist->next();
                                         return MPRemoteCommandHandlerStatusSuccess;
                                     }];
    [[commandCenter previousTrackCommand] addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent * event) {
                                             playlist->previous();
                                             return MPRemoteCommandHandlerStatusSuccess;
                                         }];
    [[commandCenter changePlaybackPositionCommand] addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent * event) {
                                                      if (!d->currentItem || [event isKindOfClass:[MPChangePlaybackPositionCommandEvent class]] == NO) {
                                                          return MPRemoteCommandHandlerStatusCommandFailed;
                                                      }
                                                      MPChangePlaybackPositionCommandEvent* positionEvent = static_cast<MPChangePlaybackPositionCommandEvent*>(event);
        d->currentItem->seek([positionEvent positionTime] * 1000);
        return MPRemoteCommandHandlerStatusSuccess;
    }];

    [[commandCenter changeShuffleModeCommand] addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent * event) {
                                                 if (!d->currentItem || [event isKindOfClass:[MPChangeShuffleModeCommandEvent class]] == NO) {
                                                     return MPRemoteCommandHandlerStatusCommandFailed;
                                                 }
                                                 MPChangeShuffleModeCommandEvent* shuffleEvent = static_cast<MPChangeShuffleModeCommandEvent*>(event);
        switch ([shuffleEvent shuffleType]) {
            case MPShuffleTypeOff:
                StateManager::instance()->playlist()->setShuffle(false);
                break;
            case MPShuffleTypeItems:
                StateManager::instance()->playlist()->setShuffle(true);
                break;
            default:
                return MPRemoteCommandHandlerStatusCommandFailed;
        }

        return MPRemoteCommandHandlerStatusSuccess;
    }];
    [[commandCenter changeRepeatModeCommand] addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent * event) {
                                                if (!d->currentItem || [event isKindOfClass:[MPChangeRepeatModeCommandEvent class]] == NO) {
                                                    return MPRemoteCommandHandlerStatusCommandFailed;
                                                }
                                                MPChangeRepeatModeCommandEvent* repeatEvent = static_cast<MPChangeRepeatModeCommandEvent*>(event);
        switch ([repeatEvent repeatType]) {
            case MPRepeatTypeOff:
                StateManager::instance()->playlist()->setRepeatOne(false);
                break;
            case MPRepeatTypeOne:
            case MPRepeatTypeAll:
                StateManager::instance()->playlist()->setRepeatOne(true);
        }

        return MPRemoteCommandHandlerStatusSuccess;
    }];
}

NowPlayingIntegration::~NowPlayingIntegration() {
    delete d;
}

void NowPlayingIntegration::updateCurrentItem() {
    if (d->currentItem) {
        d->currentItem->disconnect(this);
    }
    d->currentItem = StateManager::instance()->playlist()->currentItem();
    if (d->currentItem) {
        connect(d->currentItem, &MediaItem::metadataChanged, this, &NowPlayingIntegration::updateMetadata);
        connect(d->currentItem, &MediaItem::elapsedChanged, this, &NowPlayingIntegration::updateMetadata);
        connect(d->currentItem, &MediaItem::durationChanged, this, &NowPlayingIntegration::updateMetadata);
    }
    updateMetadata();
}

void NowPlayingIntegration::updateMetadata() {
    NSMutableDictionary<NSString*, id>* dict = [[NSMutableDictionary alloc] init];
    if (d->currentItem) {
        [dict setObject:[NSNumber numberWithInt:MPNowPlayingInfoMediaTypeAudio] forKey:MPNowPlayingInfoPropertyMediaType];
        [dict setObject:d->currentItem->metadata("url").toUrl().toNSURL() forKey:MPNowPlayingInfoPropertyAssetURL];
        [dict setObject:d->currentItem->album().toNSString() forKey:MPNowPlayingInfoCollectionIdentifier];
        [dict setObject:[NSNumber numberWithDouble:d->currentItem->elapsed() / 1000.0] forKey:MPNowPlayingInfoPropertyElapsedPlaybackTime];
        [dict setObject:[NSNumber numberWithDouble:d->currentItem->duration() / 1000.0] forKey:MPMediaItemPropertyPlaybackDuration];
        [dict setObject:[NSNumber numberWithDouble:static_cast<double>(d->currentItem->elapsed()) / d->currentItem->duration()] forKey:MPNowPlayingInfoPropertyPlaybackProgress];
        [dict setObject:[NSNumber numberWithBool:d->currentItem->duration() == 0 ? YES : NO] forKey:MPNowPlayingInfoPropertyIsLiveStream];
        [dict setObject:[NSNumber numberWithUnsignedInt:StateManager::instance()->playlist()->items().count()] forKey:MPNowPlayingInfoPropertyPlaybackQueueCount];
        [dict setObject:[NSNumber numberWithUnsignedInt:StateManager::instance()->playlist()->items().indexOf(d->currentItem)] forKey:MPNowPlayingInfoPropertyPlaybackQueueIndex];

        [dict setObject:d->currentItem->title().toNSString() forKey:MPMediaItemPropertyTitle];
        [dict setObject:QLocale().createSeparatedList(d->currentItem->authors()).toNSString() forKey:MPMediaItemPropertyArtist];
        [dict setObject:d->currentItem->album().toNSString() forKey:MPMediaItemPropertyAlbumTitle];

        if (@available(macOS 10.13.1, *)) {
            [dict setObject:QDateTime::currentDateTime().addMSecs(-d->currentItem->elapsed()).toNSDate() forKey:MPNowPlayingInfoPropertyCurrentPlaybackDate];
        }
    } else {
        [dict setObject:[NSNumber numberWithInt:MPNowPlayingInfoMediaTypeNone] forKey:MPNowPlayingInfoPropertyMediaType];
    }
    MPNowPlayingInfoCenter* center = [MPNowPlayingInfoCenter defaultCenter];
    [center setNowPlayingInfo:dict];

    switch (StateManager::instance()->playlist()->state()) {
        case Playlist::Playing:
            [center setPlaybackState:MPNowPlayingPlaybackStatePlaying];
            break;
        case Playlist::Paused:
            [center setPlaybackState:MPNowPlayingPlaybackStatePaused];
            break;
        case Playlist::Stopped:
            [center setPlaybackState:MPNowPlayingPlaybackStateStopped];
            break;
    }
}
