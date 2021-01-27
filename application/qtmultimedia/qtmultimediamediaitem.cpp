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
#include <QAudioProbe>
#include <statemanager.h>
#include <playlist.h>
#include <helpers.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <tlogger.h>
#include <visualisationmanager.h>

struct QtMultimediaMediaItemPrivate {
    QMediaPlayer* player;
    QAudioProbe* probe;
    QImage albumArt;
    QUrl url;

    QString title;
    QStringList artist;
    QString album;
    int trackNumber = 0;

    QNetworkAccessManager mgr;
};

QtMultimediaMediaItem::QtMultimediaMediaItem(QUrl url) : MediaItem() {
    d = new QtMultimediaMediaItemPrivate();
    d->url = url;

    tDebug("QtMultimediaMediaItem") << "Constructing Qt Multimedia backend item for URL" << url.toString();

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
#ifdef Q_OS_WIN
        if (error == QMediaPlayer::FormatError && d->url.isLocalFile() && QFileInfo(d->url.toLocalFile()).suffix() == "flac") {
            //Ignore
            tWarn("QtMultimediaMediaItem") << "Qt Multimedia item apparently failed with error" << error << "but since we're on Windows and this is a FLAC file we'll try anyway...";
            return;
        }
#endif


        tWarn("QtMultimediaMediaItem") << "Qt Multimedia item" << url.toString() << "failed with error" << error;
        emit this->error();
    });
    updateAlbumArt();

    connect(StateManager::instance()->playlist(), &Playlist::volumeChanged, this, [ = ] {
        d->player->setVolume(QAudio::convertVolume(StateManager::instance()->playlist()->volume(), QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale) * 100);
    });
    d->player->setVolume(QAudio::convertVolume(StateManager::instance()->playlist()->volume(), QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale) * 100);

    d->probe = new QAudioProbe(this);
    d->probe->setSource(d->player);
    connect(d->probe, &QAudioProbe::audioBufferProbed, this, [ = ](QAudioBuffer buffer) {
        QAudioFormat format = buffer.format();
        if (format.sampleSize() == 16 && format.sampleType() == QAudioFormat::SignedInt) {
            QVector<qint16> bufferData;
            if (format.channelCount() == 2) {
                bufferData.reserve(buffer.sampleCount());
                for (qint64 i = 0; i < buffer.sampleCount(); i += 2) {
                    qint16 sample = static_cast<qint16*>(buffer.data())[i] / 2 + static_cast<qint16*>(buffer.data())[i + 1] / 2;
                    bufferData.append(sample);
                }
            } else {
                bufferData.fill(0, buffer.sampleCount());
                memcpy(bufferData.data(), buffer.constData(), buffer.byteCount());
            }

            VisualisationManager::instance()->provideSamples(bufferData.toList());
        }
    });

    updateTaglib();
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

void QtMultimediaMediaItem::updateTaglib() {
    if (!d->url.isLocalFile()) return;
#ifdef Q_OS_WIN
    TagLib::FileRef file(reinterpret_cast<const wchar_t*>(d->url.toLocalFile().constData()));
#else
    TagLib::FileRef file(d->url.toLocalFile().toUtf8());
#endif
    TagLib::Tag* tag = file.tag();

    if (!tag) return;

    d->artist.clear();
    d->title = QString::fromStdString(tag->title().to8Bit());
    d->artist.append(QString::fromStdString(tag->artist().to8Bit()));
    d->album = QString::fromStdString(tag->album().to8Bit());
    d->trackNumber = tag->track();

    emit metadataChanged();
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
    } else if (!d->title.isEmpty()) {
        return d->title;
    }
    if (d->url.isLocalFile()) {
        QFileInfo file(d->url.toLocalFile());
        return file.baseName();
    } else {
        return d->url.toString();
    }
}

QStringList QtMultimediaMediaItem::authors() {
    if (d->player->availableMetaData().contains(QMediaMetaData::Author)) {
        QStringList data = d->player->metaData(QMediaMetaData::Author).toStringList();
        data.append(d->player->metaData(QMediaMetaData::AlbumArtist).toString());
        data.append(d->player->metaData(QMediaMetaData::ContributingArtist).toString());
        data.removeAll("");
        data.removeDuplicates();
        return data;
    } else {
        return d->artist;
    }
}

QString QtMultimediaMediaItem::album() {
    if (d->player->availableMetaData().contains(QMediaMetaData::AlbumTitle)) {
        return d->player->metaData(QMediaMetaData::AlbumTitle).toString();
    } else {
        return d->album;
    }
}

QImage QtMultimediaMediaItem::albumArt() {
    return d->albumArt;
}

QVariant QtMultimediaMediaItem::metadata(QString key) {
    if (d->player->availableMetaData().contains(key)) {
        return d->player->metaData(key);
    } else if (key == QMediaMetaData::TrackNumber) {
        return d->trackNumber;
    } else {
        return QVariant();
    }
}
