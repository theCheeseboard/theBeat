#include "scrobbleservice.h"

#include "lastfmapiservice.h"
#include <QDateTime>
#include <playlist.h>
#include <statemanager.h>
#include <tlogger.h>

struct ScrobbleServicePrivate {
        MediaItem* currentItem = nullptr;
        bool scrobbled = false;
};

ScrobbleService::ScrobbleService(QObject* parent) :
    QObject{parent} {
    d = new ScrobbleServicePrivate();

    connect(StateManager::instance()->playlist(), &Playlist::currentItemChanged, this, &ScrobbleService::updateCurrentItem);
    connect(StateManager::instance()->playlist(), &Playlist::stateChanged, this, [this](Playlist::State newState, Playlist::State oldState) {
        if (newState == Playlist::Playing) {
            tryNowPlaying();
        }
    });
    this->updateCurrentItem();
}

ScrobbleService::~ScrobbleService() {
    delete d;
}

void ScrobbleService::updateCurrentItem() {
    if (d->currentItem) {
        d->currentItem->disconnect(this);
    }
    d->currentItem = StateManager::instance()->playlist()->currentItem();
    d->scrobbled = false;
    if (d->currentItem) {
        connect(d->currentItem, &MediaItem::elapsedChanged, this, &ScrobbleService::tryScrobble);
        connect(d->currentItem, &MediaItem::durationChanged, this, &ScrobbleService::tryScrobble);
    }
}

void ScrobbleService::tryScrobble() {
    if (!d->currentItem) return;

    // Reset the scrobbled flag if we're repeating this track
    if (d->currentItem->elapsed() < 1000) d->scrobbled = false;

    if (d->scrobbled) return;

    // Check to see if the current item meets the criteria for scrobbling
    // Guidelines: https://www.last.fm/api/scrobbling#when-is-a-scrobble-a-scrobble
    if (d->currentItem->duration() < 30000) return; // Track should be > 30 seconds long

    auto scrobbleThreshold = qMin<quint64>(4 * 60000, d->currentItem->duration() / 2);
    if (d->currentItem->elapsed() < scrobbleThreshold) return; // Track should have played for at least 4 minutes or half the duration

    // Track should have valid metadata
    if (d->currentItem->authors().isEmpty() && d->currentItem->album().isEmpty()) return;

    // We're okay to scrobble this
    tDebug("ScrobbleService") << "Enqueueing " << d->currentItem->title() << " for scrobble";
    LastFmApiService::Scrobble scrobble(d->currentItem);
    LastFmApiService::pushScrobble(scrobble);

    d->scrobbled = true;
}

void ScrobbleService::tryNowPlaying() {
    if (!d->currentItem) return;

    // Send the now playing event
    LastFmApiService::Scrobble scrobble(d->currentItem);
    LastFmApiService::nowPlaying(scrobble);
}
