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
#include "playlist.h"

struct PlaylistPrivate {
    QList<MediaItem*> items;
    QList<MediaItem*> playOrder;

    Playlist::State state = Playlist::Stopped;
    MediaItem* currentItem = nullptr;
};

Playlist::Playlist(QObject* parent) : QObject(parent) {
    d = new PlaylistPrivate();
}

void Playlist::addItem(MediaItem* item) {
    d->items.append(item);
    d->playOrder.append(item);
    emit itemsChanged();

    this->play();
}

void Playlist::removeItem(MediaItem* item) {
    if (d->currentItem == item) {
        if (d->items.count() == 1) {
            clear();
            return;
        }

        next();
    }

    item->deleteLater();
    d->items.removeOne(item);
    d->playOrder.removeOne(item);
    emit itemsChanged();
}

void Playlist::clear() {
    setCurrentItem(nullptr);
    qDeleteAll(d->items);

    d->items.clear();
    d->playOrder.clear();

    State oldState = d->state;
    d->state = Playlist::Stopped;
    emit stateChanged(d->state, oldState);
    emit itemsChanged();
}

void Playlist::play() {
    if (d->playOrder.isEmpty()) return;
    if (!d->currentItem) setCurrentItem(d->playOrder.first());

    d->currentItem->play();

    if (d->state == Playlist::Playing) return;

    State oldState = d->state;
    d->state = Playlist::Playing;
    emit stateChanged(d->state, oldState);
}

void Playlist::pause() {
    if (d->state == Playlist::Paused) return;

    if (d->currentItem) {
        d->currentItem->pause();

        State oldState = d->state;
        d->state = Playlist::Paused;
        emit stateChanged(d->state, oldState);
    }
}

void Playlist::next() {
    if (!d->currentItem) {
        play();
        return;
    }

    int index = d->playOrder.indexOf(d->currentItem) + 1;
    if (index == d->playOrder.count()) index = 0;
    setCurrentItem(d->playOrder.at(index));
}

void Playlist::previous() {
    //TODO: Skip to beginning if elapsed < 5000
    if (!d->currentItem) {
        play();
        return;
    }

    if (d->currentItem->elapsed() >= 5000) {
        d->currentItem->seek(0);
        return;
    }

    int index = d->playOrder.indexOf(d->currentItem) - 1;
    if (index == -1) index = d->playOrder.count() - 1;
    setCurrentItem(d->playOrder.at(index));
}

Playlist::State Playlist::state() {
    return d->state;
}

MediaItem* Playlist::currentItem() {
    return d->currentItem;
}

void Playlist::setCurrentItem(MediaItem* item) {
    if (item == d->currentItem) {
        if (item == nullptr) return;
        item->seek(0);
        if (d->state == Playlist::Playing) item->play();
        return;
    }

    if (d->currentItem) {
        d->currentItem->pause();
        d->currentItem->disconnect(this);
    }

    d->currentItem = item;
    emit currentItemChanged(item);

    if (!item) {
        State oldState = d->state;
        d->state = Playlist::Stopped;
        emit stateChanged(d->state, oldState);
        return;
    }

    item->seek(0);

    //TODO: connect to signals
    connect(d->currentItem, &MediaItem::done, this, &Playlist::next);

    if (d->state == Playlist::Playing) {
        d->currentItem->play();
    }
}

QList<MediaItem*> Playlist::items() {
    return d->items;
}
