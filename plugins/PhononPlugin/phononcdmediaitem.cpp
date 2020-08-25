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
#include "phononcdmediaitem.h"

#include <QImage>
#include <QMediaMetaData>
#include <phonon/MediaObject>
#include <phonon/MediaController>
#include <phonon/MediaSource>
#include <phonon/AudioOutput>

#include <statemanager.h>
#include <playlist.h>

using namespace Phonon;

struct PhononCdMediaItemPrivate {
    QString blockDevice;
    TrackInfoPtr info;

    MediaObject* player;
    MediaController* controller;

    static QMultiMap<QString, PhononCdMediaItem*> items;
};

QMultiMap<QString, PhononCdMediaItem*> PhononCdMediaItemPrivate::items = QMultiMap<QString, PhononCdMediaItem*>();

PhononCdMediaItem::PhononCdMediaItem(QString blockDevice, TrackInfoPtr info) : MediaItem() {
    d = new PhononCdMediaItemPrivate();
    d->blockDevice = blockDevice;
    d->info = info;

    AudioOutput* output = new AudioOutput(Phonon::MusicCategory, this);
    d->player = new MediaObject(this);
    d->controller = new MediaController(d->player);
    createPath(d->player, output);

    d->player->setCurrentSource(MediaSource(Phonon::Cd, "/dev/" + d->blockDevice));
    d->controller->setCurrentTitle(info->track());

    connect(d->player, &MediaObject::tick, this, &PhononCdMediaItem::elapsedChanged);
    connect(d->player, &MediaObject::totalTimeChanged, this, &PhononCdMediaItem::durationChanged);
    connect(d->player, &MediaObject::finished, this, &PhononCdMediaItem::done);
    connect(d->controller, &MediaController::titleChanged, this, [ = ](int title) {
        if (title != d->info->track() + 1) d->controller->setCurrentTitle(d->info->track() + 1);
    });

    connect(StateManager::instance()->playlist(), &Playlist::volumeChanged, this, [ = ] {
        output->setVolume(StateManager::instance()->playlist()->volume());
    });
    output->setVolume(StateManager::instance()->playlist()->volume());

    d->items.insert(d->blockDevice, this);
}

PhononCdMediaItem::~PhononCdMediaItem() {
    d->items.remove(d->blockDevice, this);
    delete d;
}

void PhononCdMediaItem::blockDeviceGone(QString blockDevice) {
    QList<PhononCdMediaItem*> items = PhononCdMediaItemPrivate::items.values(blockDevice);
    for (PhononCdMediaItem* item : items) {
        StateManager::instance()->playlist()->removeItem(item);
        item->deleteLater();
    }
    PhononCdMediaItemPrivate::items.remove(blockDevice);
}

void PhononCdMediaItem::play() {
    if (d->controller->currentTitle() != d->info->track() + 1) d->controller->setCurrentTitle(d->info->track() + 1);
    d->player->play();
}

void PhononCdMediaItem::pause() {
    d->player->pause();
}

void PhononCdMediaItem::stop() {
    d->player->stop();
}

void PhononCdMediaItem::seek(quint64 ms) {
    d->player->seek(ms);
    if (ms == 0 && d->controller->currentTitle() != d->info->track()) d->controller->setCurrentTitle(d->info->track());
}

quint64 PhononCdMediaItem::elapsed() {
    return d->player->currentTime();
}

quint64 PhononCdMediaItem::duration() {
    return d->player->totalTime();
}

QString PhononCdMediaItem::title() {
    return d->info->title();
}

QStringList PhononCdMediaItem::authors() {
    return d->info->artist();
}

QString PhononCdMediaItem::album() {
    return d->info->album();
}

QImage PhononCdMediaItem::albumArt() {
    return d->info->albumArt();
}


QVariant PhononCdMediaItem::metadata(QString key) {
    if (key == QMediaMetaData::TrackNumber) {
        return d->info->track() + 1;
    }
    return QVariant();
}
