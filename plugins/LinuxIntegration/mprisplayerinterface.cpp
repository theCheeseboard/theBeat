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
#include "mprisplayerinterface.h"

#include <QDBusMessage>
#include <statemanager.h>
#include <playlist.h>
#include <QDBusConnection>
#include <QCryptographicHash>
#include <QImage>
#include <QBuffer>

struct MprisPlayerInterfacePrivate {
    MediaItem* currentItem = nullptr;
};

MprisPlayerInterface::MprisPlayerInterface(QObject* parent) : QDBusAbstractAdaptor(parent) {
    d = new MprisPlayerInterfacePrivate();

    Playlist* playlist = StateManager::instance()->playlist();

    connect(playlist, &Playlist::stateChanged, this, std::bind(&MprisPlayerInterface::propertyChanged, this, "PlaybackStatus"));
    connect(playlist, &Playlist::repeatOneChanged, this, std::bind(&MprisPlayerInterface::propertyChanged, this, "LoopStatus"));
    connect(playlist, &Playlist::shuffleChanged, this, std::bind(&MprisPlayerInterface::propertyChanged, this, "Shuffle"));
    connect(playlist, &Playlist::volumeChanged, this, std::bind(&MprisPlayerInterface::propertyChanged, this, "Volume"));

    connect(playlist, &Playlist::currentItemChanged, this, &MprisPlayerInterface::updateCurrentItem);
    updateCurrentItem();
}

MprisPlayerInterface::~MprisPlayerInterface() {
    delete d;
}

QString MprisPlayerInterface::PlaybackStatus() {
    switch (StateManager::instance()->playlist()->state()) {
        case Playlist::Playing:
            return QStringLiteral("Playing");
        case Playlist::Paused:
            return QStringLiteral("Paused");
        case Playlist::Stopped:
            return QStringLiteral("Stopped");
    }
}

QString MprisPlayerInterface::LoopStatus() {
    return StateManager::instance()->playlist()->repeatOne() ? QStringLiteral("Track") : QStringLiteral("Playlist");
}

void MprisPlayerInterface::setLoopStatus(QString loopStatus) {
    StateManager::instance()->playlist()->setRepeatOne(loopStatus == QStringLiteral("Track"));
}

double MprisPlayerInterface::Rate() {
    return 1;
}

void MprisPlayerInterface::setRate(double rate) {
    //noop
    Q_UNUSED(rate)
}

bool MprisPlayerInterface::Shuffle() {
    return StateManager::instance()->playlist()->shuffle();
}

void MprisPlayerInterface::setShuffle(bool shuffle) {
    StateManager::instance()->playlist()->setShuffle(shuffle);
}

QVariantMap MprisPlayerInterface::Metadata() {
    QVariantMap data;

    if (d->currentItem) {
        data.insert("mpris:trackid", QVariant::fromValue(this->trackPath(d->currentItem)));
        data.insert("xesam:title", d->currentItem->title());
        data.insert("xesam:artist", d->currentItem->authors());
        data.insert("xesam:album", d->currentItem->album());
        data.insert("mpris:length", static_cast<qint64>(d->currentItem->duration() * 1000));

        QString artUrl = QStringLiteral("data:image/png;base64,");

        QBuffer buf;
        buf.open(QBuffer::WriteOnly);
        d->currentItem->albumArt().save(&buf, "PNG");
        buf.close();

        artUrl.append(buf.data().toBase64(QByteArray::Base64Encoding));
        data.insert("mpris:artUrl", artUrl);
    } else {
        data.insert("mpris:trackid", QVariant::fromValue(QDBusObjectPath(QStringLiteral("/org/thesuite/thebeat/notrack"))));
    }

    return data;
}

double MprisPlayerInterface::Volume() {
    return StateManager::instance()->playlist()->volume();
}

void MprisPlayerInterface::setVolume(double volume) {
    StateManager::instance()->playlist()->setVolume(volume);
}

qint64 MprisPlayerInterface::Position() {
    if (d->currentItem) return d->currentItem->elapsed() * 1000;
    return 0;
}

double MprisPlayerInterface::MinimumRate() {
    return 1;
}

double MprisPlayerInterface::MaximumRate() {
    return 1;
}

bool MprisPlayerInterface::CanGoNext() {
    return true;
}

bool MprisPlayerInterface::CanGoPrevious() {
    return true;
}

bool MprisPlayerInterface::CanPlay() {
    return true;
}

bool MprisPlayerInterface::CanPause() {
    return true;
}

bool MprisPlayerInterface::CanSeek() {
    return true;
}

bool MprisPlayerInterface::CanControl() {
    return true;
}

void MprisPlayerInterface::Next() {
    StateManager::instance()->playlist()->next();
}

void MprisPlayerInterface::Previous() {
    StateManager::instance()->playlist()->previous();
}

void MprisPlayerInterface::Pause() {
    StateManager::instance()->playlist()->pause();
}

void MprisPlayerInterface::PlayPause() {
    switch (StateManager::instance()->playlist()->state()) {
        case Playlist::Playing:
            StateManager::instance()->playlist()->pause();
            break;
        case Playlist::Paused:
        case Playlist::Stopped:
            StateManager::instance()->playlist()->play();
            break;

    }
}

void MprisPlayerInterface::Stop() {
    StateManager::instance()->playlist()->pause();
}

void MprisPlayerInterface::Play() {
    StateManager::instance()->playlist()->play();
}

void MprisPlayerInterface::Seek(qint64 us) {
    if (StateManager::instance()->playlist()->currentItem()) {
        MediaItem* item = StateManager::instance()->playlist()->currentItem();
        qint64 pos = item->elapsed() + us / 1000;
        if (pos < 0) pos = 0;
        if (static_cast<quint64>(pos) > item->duration()) {
            Next();
            return;
        }

        StateManager::instance()->playlist()->currentItem()->seek(pos);
    }
}

void MprisPlayerInterface::SetPosition(QDBusObjectPath trackId, qint64 mu) {
    if (!d->currentItem) return;
    if (trackId == this->trackPath(d->currentItem)) {
        d->currentItem->seek(mu / 1000);
    }
}

void MprisPlayerInterface::OpenUri(QString uri) {

}

void MprisPlayerInterface::updateCurrentItem() {
    if (d->currentItem) {
        d->currentItem->disconnect(this);
    }
    d->currentItem = StateManager::instance()->playlist()->currentItem();
    if (d->currentItem) {
        connect(d->currentItem, &MediaItem::metadataChanged, this, std::bind(&MprisPlayerInterface::propertyChanged, this, "Metadata"));
        connect(d->currentItem, &MediaItem::elapsedChanged, this, [ = ] {
            emit Seeked(d->currentItem->elapsed() * 1000);
        });
    }
    propertyChanged("Metadata");
}


void MprisPlayerInterface::propertyChanged(QString property) {
    QDBusMessage signal = QDBusMessage::createSignal("/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "PropertiesChanged");
    QList<QVariant> args = {
        QStringLiteral("org.mpris.MediaPlayer2.Player"),
        QVariantMap({ //Changed properties
            {property, this->property(property.toUtf8())}
        }),
        QStringList({ //Invalidated properties
            property
        })
    };
    signal.setArguments(args);
    QDBusConnection::sessionBus().send(signal);
}

QDBusObjectPath MprisPlayerInterface::trackPath(MediaItem* item) {
    return QDBusObjectPath(QStringLiteral("/org/thesuite/thebeat/") + QCryptographicHash::hash((item->title() + item->authors().join(",")).toUtf8(), QCryptographicHash::Sha256).toHex());
}

