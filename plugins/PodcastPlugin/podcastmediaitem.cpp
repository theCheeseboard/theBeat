#include "podcastmediaitem.h"

#include "podcast.h"
#include "podcastitem.h"
#include <QImage>
#include <QUrl>
#include <statemanager.h>
#include <urlmanager.h>

struct PodcastMediaItemPrivate {
        MediaItem* parent;
        PodcastItemPtr podcastItem;
        QImage podcastArt;
};

PodcastMediaItem::PodcastMediaItem(PodcastItemPtr podcastItem) {
    d = new PodcastMediaItemPrivate();
    d->podcastItem = podcastItem;
    d->parent = StateManager::instance()->url()->itemForUrl(QUrl(podcastItem->playUrl()));
    d->podcastArt = QCoro::waitFor(podcastItem->image());

    connect(d->parent, &MediaItem::done, this, &PodcastMediaItem::done);
    connect(d->parent, &MediaItem::error, this, &PodcastMediaItem::error);
    connect(d->parent, &MediaItem::metadataChanged, this, &PodcastMediaItem::metadataChanged);
    connect(d->parent, &MediaItem::elapsedChanged, this, &PodcastMediaItem::elapsedChanged);
    connect(d->parent, &MediaItem::durationChanged, this, &PodcastMediaItem::durationChanged);
}

PodcastMediaItem::~PodcastMediaItem() {
    delete d;
}

void PodcastMediaItem::play() {
    d->parent->play();
}

void PodcastMediaItem::pause() {
    d->parent->pause();
}

void PodcastMediaItem::stop() {
    d->parent->stop();
}

void PodcastMediaItem::seek(quint64 ms) {
    d->parent->seek(ms);
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
