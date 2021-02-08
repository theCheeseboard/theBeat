#include "avplayerinstance.h"

#include <statemanager.h>
#include <playlist.h>
#include <tlogger.h>

struct AvPlayerInstancePrivate {
    static QUrl currentUrl;
};

QUrl AvPlayerInstancePrivate::currentUrl = QUrl();

AvPlayerInstance::AvPlayerInstance() {

}

#include <QTimer>
AVPlayer* AvPlayerInstance::instance() {
    static AVPlayer* player = [[AVPlayer alloc] init];

    QObject::connect(StateManager::instance()->playlist(), &Playlist::volumeChanged, [ = ](double volume) {
        [player setVolume:volume];
    });
    [player setVolume:StateManager::instance()->playlist()->volume()];

    [player setAllowsExternalPlayback:YES];

    return player;
}

void AvPlayerInstance::setCurrentItem(AVPlayerItem* item) {
    AvPlayerInstancePrivate::currentUrl = QUrl();
    [instance() replaceCurrentItemWithPlayerItem:item];
}

void AvPlayerInstance::setCurrentItem(AVAsset* asset) {
    AvPlayerInstancePrivate::currentUrl = QUrl();
    [instance() replaceCurrentItemWithPlayerItem:[AVPlayerItem playerItemWithAsset:asset]];
}

void AvPlayerInstance::setCurrentItem(QUrl url, NSObject* selector) {
    if (AvPlayerInstancePrivate::currentUrl != url) {
        AvPlayerInstancePrivate::currentUrl = url;
        AVPlayerItem* item = [AVPlayerItem playerItemWithURL:url.toNSURL()];
        [instance() replaceCurrentItemWithPlayerItem:item];

        [[NSNotificationCenter defaultCenter] addObserver:selector selector:@selector(playerDidFinishPlaying:) name:AVPlayerItemDidPlayToEndTimeNotification object:item];
        [[NSNotificationCenter defaultCenter] addObserver:selector selector:@selector(playerFailedToPlay:) name:AVPlayerItemFailedToPlayToEndTimeNotification object:item];
    }
}

QUrl AvPlayerInstance::currentUrl() {
    return AvPlayerInstancePrivate::currentUrl;
}
