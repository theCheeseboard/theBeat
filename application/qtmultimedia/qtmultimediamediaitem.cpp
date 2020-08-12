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
#include <QImage>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFileInfo>
#include <statemanager.h>
#include <playlist.h>
#include <helpers.h>

struct QtMultimediaMediaItemPrivate {
    QMediaPlayer* player;
    QImage albumArt;
    QUrl url;

    QNetworkAccessManager mgr;
};

QtMultimediaMediaItem::QtMultimediaMediaItem(QUrl url) : MediaItem() {
    d = new QtMultimediaMediaItemPrivate();
    d->url = url;

    d->player = new QMediaPlayer(this);
    d->player->setMedia(QMediaContent(url));
    connect(d->player, &QMediaPlayer::mediaStatusChanged, this, [ = ](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) emit done();
    });
    connect(d->player, QOverload<>::of(&QMediaPlayer::metaDataChanged), this, &QtMultimediaMediaItem::metadataChanged);
    connect(d->player, QOverload<>::of(&QMediaPlayer::metaDataChanged), this, &QtMultimediaMediaItem::updateAlbumArt);
    connect(d->player, &QMediaPlayer::positionChanged, this, &QtMultimediaMediaItem::elapsedChanged);
    connect(d->player, &QMediaPlayer::durationChanged, this, &QtMultimediaMediaItem::durationChanged);
    connect(d->player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this, [ = ](QMediaPlayer::Error error) {
        emit this->error();
    });
    updateAlbumArt();

    connect(StateManager::instance()->playlist(), &Playlist::volumeChanged, this, [ = ] {
        d->player->setVolume(QAudio::convertVolume(StateManager::instance()->playlist()->volume(), QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale) * 100);
    });
    d->player->setVolume(QAudio::convertVolume(StateManager::instance()->playlist()->volume(), QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale) * 100);
}

QtMultimediaMediaItem::~QtMultimediaMediaItem() {
    delete d;
}

void QtMultimediaMediaItem::updateAlbumArt() {
    if (d->player->availableMetaData().contains(QMediaMetaData::CoverArtImage)) {
        d->albumArt = d->player->metaData(QMediaMetaData::CoverArtImage).value<QImage>();
    } else if (d->player->availableMetaData().contains(QMediaMetaData::CoverArtUrlLarge)) {
        QUrl url = d->player->metaData(QMediaMetaData::CoverArtUrlLarge).toUrl();
        QNetworkReply* reply = d->mgr.get(QNetworkRequest(url));
        connect(reply, &QNetworkReply::finished, this, [ = ] {
            d->albumArt = QImage::fromData(reply->readAll());
            reply->deleteLater();

            emit metadataChanged();
        });
    } else {
        d->albumArt = Helpers::albumArt(d->url);
    }
}

void QtMultimediaMediaItem::play() {
    d->player->play();
}

void QtMultimediaMediaItem::pause() {
    d->player->pause();
}

void QtMultimediaMediaItem::stop() {
    d->player->stop();
}

void QtMultimediaMediaItem::seek(quint64 ms) {
    d->player->setPosition(ms);
}

quint64 QtMultimediaMediaItem::elapsed() {
    return d->player->position();
}

quint64 QtMultimediaMediaItem::duration() {
    return d->player->duration();
}

QString QtMultimediaMediaItem::title() {
    if (d->player->availableMetaData().contains(QMediaMetaData::Title)) {
        return d->player->metaData(QMediaMetaData::Title).toString();
    }
    if (d->url.isLocalFile()) {
        QFileInfo file(d->url.toLocalFile());
        return file.baseName();
    } else {
        return d->url.toString();
    }
}

QStringList QtMultimediaMediaItem::authors() {
    QStringList data = d->player->metaData(QMediaMetaData::Author).toStringList();
    data.append(d->player->metaData(QMediaMetaData::AlbumArtist).toString());
    data.append(d->player->metaData(QMediaMetaData::ContributingArtist).toString());
    data.removeAll("");
    data.removeDuplicates();
    return data;
}

QString QtMultimediaMediaItem::album() {
    return d->player->metaData(QMediaMetaData::AlbumTitle).toString();
}

QImage QtMultimediaMediaItem::albumArt() {
    return d->albumArt;
}

QVariant QtMultimediaMediaItem::metadata(QString key) {
    return d->player->metaData(key);
}
