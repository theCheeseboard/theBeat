#include "paranoiatrackinfo.h"

#include <QImage>

struct ParanoiaTrackInfoPrivate {
        QString title;
        QStringList artist;
        QString album;
        int track;
        QImage albumArt;
};

ParanoiaTrackInfo::ParanoiaTrackInfo() :
    QObject(nullptr), d{new ParanoiaTrackInfoPrivate()} {
}

ParanoiaTrackInfo::ParanoiaTrackInfo(int track) :
    QObject(nullptr), d{new ParanoiaTrackInfoPrivate()} {
    d->title = tr("Track %1").arg(track + 1);
    d->album = tr("Unknown");
    d->track = track;
}

ParanoiaTrackInfo::~ParanoiaTrackInfo() {
    delete d;
}

QString ParanoiaTrackInfo::title() {
    return d->title;
}

QStringList ParanoiaTrackInfo::artist() {
    return d->artist;
}

QString ParanoiaTrackInfo::album() {
    return d->album;
}

int ParanoiaTrackInfo::track() {
    return d->track;
}

QImage ParanoiaTrackInfo::albumArt() {
    return d->albumArt;
}

void ParanoiaTrackInfo::setData(QString title, QStringList artist, QString album) {
    d->title = title;
    d->artist = artist;
    d->album = album;

    emit dataChanged();
}

void ParanoiaTrackInfo::setAlbumArt(QImage albumArt) {
    d->albumArt = albumArt;
}
