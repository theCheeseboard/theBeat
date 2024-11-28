#include "gsttrackinfo.h"

#include <QImage>

struct GstTrackInfoPrivate {
        QString title;
        QStringList artist;
        QString album;
        int track;
        QImage albumArt;
};

GstTrackInfo::GstTrackInfo() :
    QObject(nullptr), d{new GstTrackInfoPrivate()} {
}

GstTrackInfo::GstTrackInfo(int track) :
    QObject(nullptr), d{new GstTrackInfoPrivate()} {
    d->title = tr("Track %1").arg(track + 1);
    d->album = tr("Unknown");
    d->track = track;
}

GstTrackInfo::~GstTrackInfo() {
    delete d;
}

QString GstTrackInfo::title() {
    return d->title;
}

QStringList GstTrackInfo::artist() {
    return d->artist;
}

QString GstTrackInfo::album() {
    return d->album;
}

int GstTrackInfo::track() {
    return d->track;
}

QImage GstTrackInfo::albumArt() {
    return d->albumArt;
}

void GstTrackInfo::setData(QString title, QStringList artist, QString album) {
    d->title = title;
    d->artist = artist;
    d->album = album;

    emit dataChanged();
}

void GstTrackInfo::setAlbumArt(QImage albumArt) {
    d->albumArt = albumArt;
}
