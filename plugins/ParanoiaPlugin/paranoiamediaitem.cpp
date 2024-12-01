#include "paranoiamediaitem.h"
#include "paranoiaplayer.h"

#include <QAudioSink>
#include <QImage>

struct ParanoiaMediaItemPrivate {
        ParanoiaPlayer::Track trackToc;
        ParanoiaTrackInfoPtr trackInfo;
        int track;
        QAudioSink* sink;
        ParanoiaPlayer* player;

        lsn_t position;
};

ParanoiaMediaItem::ParanoiaMediaItem(ParanoiaPlayer* player, int track, QAudioSink* sink, ParanoiaTrackInfoPtr trackInfo) :
    MediaItem{}, d{new ParanoiaMediaItemPrivate} {
    d->player = player;
    d->track = track;
    d->sink = sink;
    d->trackInfo = trackInfo;
    d->trackToc = player->track(track);

    connect(player, &ParanoiaPlayer::frameRead, this, &ParanoiaMediaItem::frameRead);
}

ParanoiaMediaItem::~ParanoiaMediaItem() {
    delete d;
}

void ParanoiaMediaItem::frameRead(lsn_t position) {
    d->position = position;
    if (d->position == d->trackToc.lastLsn) {
        emit done();
    }
    emit elapsedChanged();
}

void ParanoiaMediaItem::play() {
    if (d->sink->state() != QAudio::SuspendedState) return;
    d->sink->resume();
}

void ParanoiaMediaItem::pause() {
    if (d->sink->state() == QAudio::SuspendedState) return;
    d->sink->suspend();
}

void ParanoiaMediaItem::stop() {
    if (d->sink->state() == QAudio::SuspendedState) return;
    d->sink->suspend();
}

void ParanoiaMediaItem::seek(quint64 ms) {
    d->player->seek(d->track, ms);
}

quint64 ParanoiaMediaItem::elapsed() {
    auto elapsed = (d->position - d->trackToc.firstLsn) * 13.333;
    if (elapsed < 0) return 0;
    return elapsed;
}

quint64 ParanoiaMediaItem::duration() {
    return (d->trackToc.lastLsn - d->trackToc.firstLsn) * 13.333;
}

QImage ParanoiaMediaItem::albumArt() {
    return {};
}

QString ParanoiaMediaItem::lyrics() {
    return {};
}

QString ParanoiaMediaItem::lyricFormat() {
    return {};
}

QString ParanoiaMediaItem::title() {
    return d->trackInfo->title();
}

QVariant ParanoiaMediaItem::metadata(QString key) {
    if (key == "TrackNumber") {
        return d->track + 1;
    }

    return {};
}

QStringList ParanoiaMediaItem::authors() {
    return d->trackInfo->artist();
}

QString ParanoiaMediaItem::album() {
    return d->trackInfo->album();
}
