/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "trackinfo.h"

#include <QImage>

struct TrackInfoPrivate {
    QString title;
    QStringList artist;
    QString album;
    int track;
    QImage albumArt;
};

TrackInfo::TrackInfo() : QObject(nullptr) {
    d = new TrackInfoPrivate();

}

TrackInfo::TrackInfo(int track) {
    d = new TrackInfoPrivate();
    d->title = tr("Track %1").arg(track + 1);
    d->album = tr("Unknown");
    d->track = track;
}

TrackInfo::~TrackInfo() {
    delete d;
}

QString TrackInfo::title() {
    return d->title;
}

QStringList TrackInfo::artist() {
    return d->artist;
}

QString TrackInfo::album() {
    return d->album;
}

int TrackInfo::track() {
    return d->track;
}

QImage TrackInfo::albumArt() {
    return d->albumArt;
}

void TrackInfo::setData(QString title, QStringList artist, QString album) {
    d->title = title;
    d->artist = artist;
    d->album = album;

    emit dataChanged();
}

void TrackInfo::setAlbumArt(QImage albumArt) {
    d->albumArt = albumArt;
}
