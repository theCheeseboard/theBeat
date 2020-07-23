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
#include "qtmultimediamediaitem.h"

#include <QMediaPlayer>
#include <QMediaMetaData>

struct QtMultimediaMediaItemPrivate {
    QMediaPlayer* player;
};

QtMultimediaMediaItem::QtMultimediaMediaItem(QUrl url) : MediaItem() {
    d = new QtMultimediaMediaItemPrivate();

    d->player = new QMediaPlayer(this);
    d->player->setMedia(QMediaContent(url));
    connect(d->player, &QMediaPlayer::mediaStatusChanged, this, [ = ](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) emit done();
    });
    connect(d->player, QOverload<>::of(&QMediaPlayer::metaDataChanged), this, &QtMultimediaMediaItem::metadataChanged);
}

QtMultimediaMediaItem::~QtMultimediaMediaItem() {
    delete d;
}

void QtMultimediaMediaItem::play() {
    d->player->play();
}

void QtMultimediaMediaItem::pause() {
    d->player->pause();
}

void QtMultimediaMediaItem::seek(quint64 ms) {
    d->player->setPosition(ms);
}

QString QtMultimediaMediaItem::title() {
    return d->player->metaData(QMediaMetaData::Title).toString();
}

QStringList QtMultimediaMediaItem::authors() {
    return d->player->metaData(QMediaMetaData::Author).toStringList();
}

QString QtMultimediaMediaItem::album() {
    return d->player->metaData(QMediaMetaData::AlbumTitle).toString();
}
