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
#include "maccdmediaitem.h"

#include <QImage>
#include <QMediaMetaData>
#include <QVariant>
#include <visualisationmanager.h>

#include <QAudioOutput>
#include <QMediaPlayer>

#include <QDir>
#include <playlist.h>
#include <statemanager.h>
#include <tlogger.h>

struct MacCdMediaItemPrivate {
        static QAudioOutput* audioOutput;
        TrackInfoPtr info;
        QString volume;

        QMediaPlayer* player;

        static QMultiMap<QString, MacCdMediaItem*> items;
};

QAudioOutput* MacCdMediaItemPrivate::audioOutput = nullptr;

QMultiMap<QString, MacCdMediaItem*> MacCdMediaItemPrivate::items = QMultiMap<QString, MacCdMediaItem*>();

MacCdMediaItem::MacCdMediaItem(QString volume, TrackInfoPtr info) :
    MediaItem() {
    d = new MacCdMediaItemPrivate();
    d->volume = volume;
    d->info = info;

    if (!MacCdMediaItemPrivate::audioOutput) {
        MacCdMediaItemPrivate::audioOutput = new QAudioOutput();

        connect(StateManager::instance()->playlist(), &Playlist::volumeChanged, MacCdMediaItemPrivate::audioOutput, [=] {
            MacCdMediaItemPrivate::audioOutput->setVolume(QAudio::convertVolume(StateManager::instance()->playlist()->volume(), QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale) * 100);
        });
        MacCdMediaItemPrivate::audioOutput->setVolume(QAudio::convertVolume(StateManager::instance()->playlist()->volume(), QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale) * 100);
    }

    // Locate the track to be played
    QDir dir(volume);
    QUrl url;
    for (const QString& track : dir.entryList(QDir::Files)) {
        if (track.startsWith(QStringLiteral("%1 ").arg(info->track() + 1))) {
            url = QUrl::fromLocalFile(dir.absoluteFilePath(track));
        }
    }

    tDebug("MacCdMediaItem") << "Constructing Mac CD backend item for URL" << url.toString();

    d->player = new QMediaPlayer(this);
    d->player->setSource(url);
    connect(d->player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) emit done();
    });
    connect(d->player, &QMediaPlayer::positionChanged, this, &MacCdMediaItem::elapsedChanged);
    connect(d->player, &QMediaPlayer::durationChanged, this, &MacCdMediaItem::durationChanged);
    connect(d->player, &QMediaPlayer::errorOccurred, this, [this, url](QMediaPlayer::Error error, QString errorString) {
        tWarn("QtMultimediaMediaItem") << "Mac CD item" << url.toString() << "failed with error" << error;
        emit this->error();
    });
    d->player->setAudioOutput(MacCdMediaItemPrivate::audioOutput);

    d->items.insert(d->volume, this);
}

MacCdMediaItem::~MacCdMediaItem() {
    d->items.remove(d->volume, this);
    delete d;
}

void MacCdMediaItem::volumeGone(QString volume) {
    QList<MacCdMediaItem*> items = MacCdMediaItemPrivate::items.values(volume);
    for (MacCdMediaItem* item : items) {
        StateManager::instance()->playlist()->removeItem(item);
        item->deleteLater();
    }
    MacCdMediaItemPrivate::items.remove(volume);
}

void MacCdMediaItem::play() {
    d->player->play();
}

void MacCdMediaItem::pause() {
    d->player->pause();
}

void MacCdMediaItem::stop() {
    d->player->stop();
}

void MacCdMediaItem::seek(quint64 ms) {
    d->player->setPosition(ms);
}

quint64 MacCdMediaItem::elapsed() {
    return d->player->position();
}

quint64 MacCdMediaItem::duration() {
    return d->player->duration();
}

QString MacCdMediaItem::title() {
    return d->info->title();
}

QStringList MacCdMediaItem::authors() {
    return d->info->artist();
}

QString MacCdMediaItem::album() {
    return d->info->album();
}

QImage MacCdMediaItem::albumArt() {
    return d->info->albumArt();
}

QVariant MacCdMediaItem::metadata(QMediaMetaData::Key key) {
    if (key == QMediaMetaData::TrackNumber) {
        return d->info->track() + 1;
    }
    return QVariant();
}

QString MacCdMediaItem::lyrics() {
    return "";
}

QString MacCdMediaItem::lyricFormat() {
    return "";
}
