#include "podcastmediaitem.h"

#include "podcast.h"
#include "podcastitem.h"
#include <QImage>
#include <QTimer>
#include <QUrl>
#include <statemanager.h>
#include <urlmanager.h>

struct PodcastMediaItemPrivate {
        MediaItem* parent;
        PodcastItemPtr podcastItem;
        QImage podcastArt;
        QTimer* elapsedUpdateTimer;
        bool initialPlay = false;
        quint64 initialPlayed;

        bool playDone = false;
};

PodcastMediaItem::PodcastMediaItem(PodcastItemPtr podcastItem) {
    d = new PodcastMediaItemPrivate();
    d->podcastItem = podcastItem;
    d->parent = StateManager::instance()->url()->itemForUrl(QUrl(podcastItem->playUrl()));
    d->podcastArt = QCoro::waitFor(podcastItem->image());
    d->initialPlayed = d->podcastItem->played();

    // Don't resume the podcast if we've already completed listening to it
    d->initialPlay = d->podcastItem->isCompleted();

    d->elapsedUpdateTimer = new QTimer(this);
    d->elapsedUpdateTimer->setInterval(10000);
    connect(d->elapsedUpdateTimer, &QTimer::timeout, this, &PodcastMediaItem::updatePodcastElapsed);
    d->elapsedUpdateTimer->start();
    connect(d->parent, &MediaItem::done, this, [this] {
        this->updatePodcastElapsed();

        // Stop any more progress updates in case the podcast restarts
        d->playDone = true;
    });

    connect(d->parent, &MediaItem::done, this, &PodcastMediaItem::done);
    connect(d->parent, &MediaItem::error, this, &PodcastMediaItem::error);
    connect(d->parent, &MediaItem::metadataChanged, this, &PodcastMediaItem::metadataChanged);
    connect(d->parent, &MediaItem::elapsedChanged, this, &PodcastMediaItem::elapsedChanged);
    connect(d->parent, &MediaItem::durationChanged, this, &PodcastMediaItem::durationChanged);

    connect(d->parent, &MediaItem::elapsedChanged, this, [this] {
        if (!d->initialPlay && d->parent->elapsed() != 0) {
            d->initialPlay = true;
            d->parent->seek(d->initialPlayed * 1000);
        }
    });
}

PodcastMediaItem::~PodcastMediaItem() {
    delete d;
}

void PodcastMediaItem::updatePodcastElapsed() {
    if (!d->playDone) {
        d->podcastItem->setPlayed(this->elapsed() / 1000);
    }
}

void PodcastMediaItem::play() {
    d->parent->play();
}

void PodcastMediaItem::pause() {
    d->parent->pause();
    updatePodcastElapsed();
}

void PodcastMediaItem::stop() {
    d->parent->stop();
}

void PodcastMediaItem::seek(quint64 ms) {
    d->parent->seek(ms);
    updatePodcastElapsed();
}

quint64 PodcastMediaItem::elapsed() {
    return d->parent->elapsed();
}

quint64 PodcastMediaItem::duration() {
    return d->parent->duration();
}

QString PodcastMediaItem::title() {
    return d->podcastItem->title();
}

QStringList PodcastMediaItem::authors() {
    return {d->podcastItem->creator()};
}

QString PodcastMediaItem::album() {
    return d->podcastItem->parentPodcast()->name();
}

QImage PodcastMediaItem::albumArt() {
    return d->podcastArt;
}

QString PodcastMediaItem::lyrics() {
    return "";
}

QString PodcastMediaItem::lyricFormat() {
    return "";
}
