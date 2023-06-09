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

#include <QRandomGenerator>
#include <tnotification.h>

struct PlaylistPrivate {
        QList<MediaItem*> items;
        QList<MediaItem*> playOrder;

        Playlist::State state = Playlist::Stopped;
        MediaItem* currentItem = nullptr;

        bool repeatOne = false;
        bool repeatAll = true;
        bool shuffle = false;
        bool pauseAfterCurrentTrack = false;
        double volume = 1;

        bool trackChangeNotificationsEnabled = true;

        QString oldTitle;
};

Playlist::Playlist(QObject* parent) :
    QObject(parent) {
    d = new PlaylistPrivate();
}

void Playlist::addItem(MediaItem* item) {
    d->items.append(item);

    if (d->shuffle) {
        // Add this item somewhere random in the play order
        d->playOrder.insert(QRandomGenerator::global()->bounded(d->playOrder.count() + 1), item);
    } else {
        d->playOrder.append(item);
    }
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

void Playlist::insertItem(int index, MediaItem* item) {
    d->items.insert(index, item);
    if (d->shuffle) {
        // Add this item somewhere random in the play order
        d->playOrder.insert(QRandomGenerator::global()->bounded(d->playOrder.count() + 1), item);
    } else {
        d->playOrder.insert(index, item);
    }
    emit itemsChanged();
}

MediaItem* Playlist::takeItem(int index) {
    // This is used exclusively for drag and drop, so we'll see it again soon
    MediaItem* item = d->items.takeAt(index);
    d->playOrder.removeOne(item);
    emit itemsChanged();
    return item;
}

void Playlist::clear() {
    setCurrentItem(nullptr);
    for (MediaItem* item : d->items) {
        item->deleteLater();
    }

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

void Playlist::playPause() {
    switch (state()) {
        case Playlist::Playing:
            pause();
            break;
        case Playlist::Paused:
        case Playlist::Stopped:
            play();
            break;
    }
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

    // Repeat this track if repeat one is on
    if (d->repeatOne) {
        d->currentItem->seek(0);
        d->currentItem->play();
        return;
    }

    bool pauseAfterCurrentTrack = d->pauseAfterCurrentTrack;

    int index = d->playOrder.indexOf(d->currentItem) + 1;
    if (index == d->playOrder.count()) {
        if (!d->repeatAll) pause();
        index = 0;
    }
    setCurrentItem(d->playOrder.at(index));

    // Pause now if pause after current track is enabled
    if (pauseAfterCurrentTrack) {
        pause();
        // Will have been disabled in setCurrentItem()
    }
}

void Playlist::previous() {
    // TODO: Skip to beginning if elapsed < 5000
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

    // Disable pause after current track
    setPauseAfterCurrentTrack(false);
}

void Playlist::updateMetadata() {
    if (!d->currentItem) return;
    if (d->currentItem->title() != d->oldTitle) {
        if (!d->currentItem->title().isEmpty()) {
            // Fire a notification
            QStringList text = {
                d->currentItem->title(),
                QLocale().createSeparatedList(d->currentItem->authors())};
            text.removeAll("");

            if (d->trackChangeNotificationsEnabled) {
                tNotification* notification = new tNotification();
                notification->setSummary(tr("Now Playing"));
                notification->setText(text.join(" · "));
                notification->setTransient(true);
                notification->setSoundOn(false);
                notification->insertAction(QStringLiteral("next"), tr("Skip Next"));
                connect(notification, &tNotification::actionClicked, this, [this](QString key) {
                    if (key == QStringLiteral("next")) {
                        // Skip to the next track
                        this->next();
                    }
                });
                notification->post();
            }
        }

        d->oldTitle = d->currentItem->title();
    }
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

    // TODO: connect to signals
    connect(d->currentItem, &MediaItem::done, this, &Playlist::next);
    connect(d->currentItem, &MediaItem::error, this, [this, item] {
        tNotification* notification = new tNotification();
        notification->setSummary(tr("Playback Failed"));
        notification->setText(tr("\"%1\" was removed from the play queue because it couldn't be played.").arg(item->title()));
        notification->setTransient(true);
        notification->setSoundOn(false);
        notification->post();

        // Remove the item from the playlist because it can't be played
        removeItem(item);
    });
    connect(d->currentItem, &MediaItem::metadataChanged, this, &Playlist::updateMetadata);

    if (d->state == Playlist::Playing) {
        d->currentItem->play();
    }

    updateMetadata();

    // Disable pause after current track
    setPauseAfterCurrentTrack(false);
}

QList<MediaItem*> Playlist::items() {
    return d->items;
}

void Playlist::setRepeatOne(bool repeatOne) {
    d->repeatOne = repeatOne;
    emit repeatOneChanged(repeatOne);
}

void Playlist::setShuffle(bool shuffle) {
    d->shuffle = shuffle;

    if (shuffle) {
        // Shuffle the play order
        QList<MediaItem*> remaining = d->items;
        d->playOrder.clear();
        while (!remaining.isEmpty()) {
            int index = QRandomGenerator::global()->bounded(remaining.count());
            d->playOrder.append(remaining.at(index));
            remaining.removeAt(index);
        }
    } else {
        // Reset the play order
        d->playOrder = d->items;
    }

    emit shuffleChanged(shuffle);
}

bool Playlist::repeatOne() {
    return d->repeatOne;
}

void Playlist::setRepeatAll(bool repeatAll) {
    d->repeatAll = repeatAll;
    emit repeatAllChanged(repeatAll);
}

bool Playlist::repeatAll() {
    return d->repeatAll;
}

bool Playlist::shuffle() {
    return d->shuffle;
}

void Playlist::setVolume(double volume) {
    d->volume = volume;
    emit volumeChanged(volume);
    emit logAdjustedVolumeChanged(this->logAdjustedVolume());
}

double Playlist::volume() {
    return d->volume;
}

double Playlist::logAdjustedVolume() {
    if (d->volume > 0.99) return 1;
    return -std::log(1 - d->volume) / std::log(100);
}

void Playlist::setTrachChangeNotificationsEnabled(bool notificationsEnabled) {
    d->trackChangeNotificationsEnabled = notificationsEnabled;
}

void Playlist::setPauseAfterCurrentTrack(bool pauseAfterCurrentTrack) {
    d->pauseAfterCurrentTrack = pauseAfterCurrentTrack;
    emit pauseAfterCurrentTrackChanged(pauseAfterCurrentTrack);
}

bool Playlist::pauseAfterCurrentTrack() {
    return d->pauseAfterCurrentTrack;
}
