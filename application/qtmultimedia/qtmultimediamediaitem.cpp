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

#include "library/librarymanager.h"
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QMediaMetaData>
#include <QMediaPlayer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAudioOutput>
#include <helpers.h>
#include <playlist.h>
#include <statemanager.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <tlogger.h>
#include <visualisationmanager.h>

struct QtMultimediaMediaItemPrivate {
  static QAudioOutput* audioOutput;
        QMediaPlayer* player = nullptr;
//        QAudioProbe* probe;
        QImage albumArt;
        QUrl url;

        QString title;
        QStringList artist;
        QString album;
        int trackNumber = 0;
        QString lyrics;

        bool firstPlay = true;

        QNetworkAccessManager mgr;
};

QAudioOutput* QtMultimediaMediaItemPrivate::audioOutput = nullptr;

QtMultimediaMediaItem::QtMultimediaMediaItem(QUrl url) :
    MediaItem() {
    d = new QtMultimediaMediaItemPrivate();
    d->url = url;

    if (!QtMultimediaMediaItemPrivate::audioOutput) {
        QtMultimediaMediaItemPrivate::audioOutput = new QAudioOutput();

        connect(StateManager::instance()->playlist(), &Playlist::volumeChanged, QtMultimediaMediaItemPrivate::audioOutput, [=] {
          QtMultimediaMediaItemPrivate::audioOutput->setVolume(QAudio::convertVolume(StateManager::instance()->playlist()->volume(), QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale) * 100);
        });
        QtMultimediaMediaItemPrivate::audioOutput->setVolume(QAudio::convertVolume(StateManager::instance()->playlist()->volume(), QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale) * 100);
    }

    tDebug("QtMultimediaMediaItem") << "Constructing Qt Multimedia backend item for URL" << url.toString();

    updateAlbumArt();
    updateTaglib();
    loadLyrics();
}

QtMultimediaMediaItem::~QtMultimediaMediaItem() {
    delete d;
}

void QtMultimediaMediaItem::preparePlayer() {
    if (d->player) return;

    tDebug("QtMultimediaMediaItem") << "Preparing Qt Multimedia player for URL" << d->url.toString();

    d->player = new QMediaPlayer(this);
    d->player->setSource(d->url);
    connect(d->player, &QMediaPlayer::mediaStatusChanged, this, [=](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) emit done();
    });
    connect(d->player, QOverload<>::of(&QMediaPlayer::metaDataChanged), this, &QtMultimediaMediaItem::metadataChanged);
    connect(d->player, QOverload<>::of(&QMediaPlayer::metaDataChanged), this, &QtMultimediaMediaItem::updateAlbumArt);
    connect(d->player, &QMediaPlayer::positionChanged, this, &QtMultimediaMediaItem::elapsedChanged);
    connect(d->player, &QMediaPlayer::durationChanged, this, &QtMultimediaMediaItem::durationChanged);
    connect(d->player, &QMediaPlayer::errorOccurred, this, [=](QMediaPlayer::Error error, QString errorString) {
#ifdef Q_OS_WIN
        if (error == QMediaPlayer::FormatError && d->url.isLocalFile() && QFileInfo(d->url.toLocalFile()).suffix() == "flac") {
            // Ignore
            tWarn("QtMultimediaMediaItem") << "Qt Multimedia item apparently failed with error" << error << "but since we're on Windows and this is a FLAC file we'll try anyway...";
            return;
        }
#endif

        tWarn("QtMultimediaMediaItem") << "Qt Multimedia item" << d->url.toString() << "failed with error" << error << "description" << errorString;
        emit this->error();
    });
    updateAlbumArt();

    //TODO: Audio Probe
//    d->probe = new QAudioProbe(this);
//    d->probe->setSource(d->player);
//    connect(d->probe, &QAudioProbe::audioBufferProbed, this, [=](QAudioBuffer buffer) {
//        QAudioFormat format = buffer.format();
//        if (format.sampleSize() == 16 && format.sampleType() == QAudioFormat::SignedInt) {
//            QVector<qint16> bufferData;
//            if (format.channelCount() == 2) {
//                bufferData.reserve(buffer.sampleCount());
//                for (qint64 i = 0; i < buffer.sampleCount(); i += 2) {
//                    qint16 sample = static_cast<qint16*>(buffer.data())[i] / 2 + static_cast<qint16*>(buffer.data())[i + 1] / 2;
//                    bufferData.append(sample);
//                }
//            } else {
//                bufferData.fill(0, buffer.sampleCount());
//                memcpy(bufferData.data(), buffer.constData(), buffer.byteCount());
//            }
//
//            StateManager::instance()->visualisation()->provideSamples(bufferData.toList());
//        } else if (format.sampleSize() == 32 && format.sampleType() == QAudioFormat::Float) {
//            QVector<qint16> bufferData;
//            bufferData.reserve(buffer.sampleCount());
//            if (format.channelCount() == 2) {
//                for (qint64 i = 0; i < buffer.sampleCount(); i += 2) {
//                    float sample = static_cast<float*>(buffer.data())[i] / 2 + static_cast<float*>(buffer.data())[i + 1] / 2;
//                    sample = sample * 32768;
//                    if (sample > 32767) sample = 32767;
//                    if (sample < -32768) sample = -32768;
//                    bufferData.append(static_cast<qint16>(sample));
//                    bufferData.append(sample);
//                }
//            } else {
//                for (qint64 i = 0; i < buffer.sampleCount(); i++) {
//                    float sample = static_cast<float*>(buffer.data())[i];
//                    sample = sample * 32768;
//                    if (sample > 32767) sample = 32767;
//                    if (sample < -32768) sample = -32768;
//                    bufferData.append(static_cast<qint16>(sample));
//                }
//            }
//
//            StateManager::instance()->visualisation()->provideSamples(bufferData.toList());
//        } else {
//            tDebug("QtMultimediaMediaItem") << "Weird format:";
//            tDebug("QtMultimediaMediaItem") << "Sample size " << format.sampleSize() << "; Sample type " << format.sampleType() << "; Channels " << format.channelCount();
//        }
//    });
}

void QtMultimediaMediaItem::updateAlbumArt() {
    if (d->player && d->player->metaData().keys().contains(QMediaMetaData::CoverArtImage)) {
        d->albumArt = d->player->metaData().value(QMediaMetaData::CoverArtImage).value<QImage>();
//    } else if (d->player && d->player->metaData().keys().contains(QMediaMetaData::CoverArtUrlLarge)) {
//        QUrl url = d->player->metaData().value(QMediaMetaData::CoverArtUrlLarge).toUrl();
//        QNetworkReply* reply = d->mgr.get(QNetworkRequest(url));
//        connect(reply, &QNetworkReply::finished, this, [=] {
//            d->albumArt = QImage::fromData(reply->readAll());
//            reply->deleteLater();
//
//            emit metadataChanged();
//        });
    } else {
        Helpers::albumArt(d->url).then([=](QImage image) {
            d->albumArt = image;
            emit metadataChanged();
        });
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
    d->title = QString::fromWCharArray(tag->title().toCWString());
    d->artist.append(QString::fromWCharArray(tag->artist().toCWString()));
    d->album = QString::fromWCharArray(tag->album().toCWString());
    d->trackNumber = tag->track();

    emit metadataChanged();
}

void QtMultimediaMediaItem::loadLyrics() {
    if (!d->url.isLocalFile()) return;

    QFileInfo file(d->url.toLocalFile());
    QFile lyricFile(file.dir().absoluteFilePath(file.completeBaseName() + ".lrc"));
    if (lyricFile.exists()) {
        lyricFile.open(QFile::ReadOnly);
        d->lyrics = lyricFile.readAll();
        lyricFile.close();
    }
}

void QtMultimediaMediaItem::play() {
    preparePlayer();
    d->player->setAudioOutput(QtMultimediaMediaItemPrivate::audioOutput);
    d->player->play();
}

void QtMultimediaMediaItem::pause() {
    preparePlayer();
    d->player->pause();
}

void QtMultimediaMediaItem::stop() {
    preparePlayer();
    d->player->stop();
}

void QtMultimediaMediaItem::seek(quint64 ms) {
    preparePlayer();
    if (ms == 0 && d->url.isLocalFile() && (d->player->position() != 0 || d->firstPlay)) {
        LibraryManager::instance()->bumpTrackPlayCount(d->url.toLocalFile());
        d->firstPlay = false;
    }
    d->player->setPosition(ms);
}

quint64 QtMultimediaMediaItem::elapsed() {
    if (!d->player) return 0;
    return d->player->position();
}

quint64 QtMultimediaMediaItem::duration() {
    preparePlayer();
    return d->player->duration();
}

QString QtMultimediaMediaItem::title() {
    if (d->player && d->player->metaData().keys().contains(QMediaMetaData::Title)) {
        return d->player->metaData().value(QMediaMetaData::Title).toString();
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
    if (d->player && d->player->metaData().keys().contains(QMediaMetaData::Author)) {
        QStringList data = d->player->metaData().value(QMediaMetaData::Author).toStringList();
        data.append(d->player->metaData().value(QMediaMetaData::AlbumArtist).toString());
        data.append(d->player->metaData().value(QMediaMetaData::ContributingArtist).toString());
        data.removeAll("");
        data.removeDuplicates();
        return data;
    } else {
        return d->artist;
    }
}

QString QtMultimediaMediaItem::album() {
    if (d->player && d->player->metaData().keys().contains(QMediaMetaData::AlbumTitle)) {
        return d->player->metaData().value(QMediaMetaData::AlbumTitle).toString();
    } else {
        return d->album;
    }
}

QImage QtMultimediaMediaItem::albumArt() {
    return d->albumArt;
}

QVariant QtMultimediaMediaItem::metadata(QString key) {
    if (d->player && d->player->metaData().keys().contains(Helpers::metadataKeyForString(key))) {
        return d->player->metaData().value(Helpers::metadataKeyForString(key));
    } else if (key == Helpers::stringForMetadataKey(QMediaMetaData::TrackNumber)) {
        return d->trackNumber;
    } else {
        return QVariant();
    }
}

QString QtMultimediaMediaItem::lyrics() {
    return d->lyrics;
}

QString QtMultimediaMediaItem::lyricFormat() {
    if (d->lyrics.isEmpty()) return "";
    return "lrc";
}
