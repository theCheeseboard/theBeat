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
#include "mprisplayer.h"

#include <QDBusMessage>
#include <statemanager.h>
#include <playlist.h>
#include <QDBusConnection>
#include <QCryptographicHash>
#include <QImage>
#include <QBuffer>

struct MprisPlayerPrivate {
    MediaItem* currentItem = nullptr;
};

MprisPlayer::MprisPlayer(QObject* parent) : QDBusAbstractAdaptor(parent) {
    d = new MprisPlayerPrivate();

    Playlist* playlist = StateManager::instance()->playlist();

    connect(playlist, &Playlist::stateChanged, this, std::bind(&MprisPlayer::propertyChanged, this, "PlaybackStatus"));
    connect(playlist, &Playlist::repeatOneChanged, this, std::bind(&MprisPlayer::propertyChanged, this, "LoopStatus"));
    connect(playlist, &Playlist::shuffleChanged, this, std::bind(&MprisPlayer::propertyChanged, this, "Shuffle"));
    connect(playlist, &Playlist::volumeChanged, this, std::bind(&MprisPlayer::propertyChanged, this, "Volume"));

    connect(playlist, &Playlist::currentItemChanged, this, &MprisPlayer::updateCurrentItem);
    updateCurrentItem();
}

MprisPlayer::~MprisPlayer() {
    delete d;
}

QString MprisPlayer::PlaybackStatus() {
    switch (StateManager::instance()->playlist()->state()) {
        case Playlist::Playing:
            return QStringLiteral("Playing");
        case Playlist::Paused:
            return QStringLiteral("Paused");
        case Playlist::Stopped:
            return QStringLiteral("Stopped");
    }
}

QString MprisPlayer::LoopStatus() {
    return StateManager::instance()->playlist()->repeatOne() ? QStringLiteral("Track") : QStringLiteral("Playlist");
}

void MprisPlayer::setLoopStatus(QString loopStatus) {
    StateManager::instance()->playlist()->setRepeatOne(loopStatus == QStringLiteral("Track"));
}

double MprisPlayer::Rate() {
    return 1;
}

void MprisPlayer::setRate(double rate) {
    //noop
    Q_UNUSED(rate)
}

bool MprisPlayer::Shuffle() {
    return StateManager::instance()->playlist()->shuffle();
}

void MprisPlayer::setShuffle(bool shuffle) {
    StateManager::instance()->playlist()->setShuffle(shuffle);
}

QVariantMap MprisPlayer::Metadata() {
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

double MprisPlayer::Volume() {
    return StateManager::instance()->playlist()->volume();
}

void MprisPlayer::setVolume(double volume) {
    StateManager::instance()->playlist()->setVolume(volume);
}

qint64 MprisPlayer::Position() {
    if (d->currentItem) return d->currentItem->elapsed() * 1000;
    return 0;
}

double MprisPlayer::MinimumRate() {
    return 1;
}

double MprisPlayer::MaximumRate() {
    return 1;
}

bool MprisPlayer::CanGoNext() {
    return true;
}

bool MprisPlayer::CanGoPrevious() {
    return true;
}

bool MprisPlayer::CanPlay() {
    return true;
}

bool MprisPlayer::CanPause() {
    return true;
}

bool MprisPlayer::CanSeek() {
    return true;
}

bool MprisPlayer::CanControl() {
    return true;
}

void MprisPlayer::Next() {
    StateManager::instance()->playlist()->next();
}

void MprisPlayer::Previous() {
    StateManager::instance()->playlist()->previous();
}

void MprisPlayer::Pause() {
    StateManager::instance()->playlist()->pause();
}

void MprisPlayer::PlayPause() {
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

void MprisPlayer::Stop() {
    StateManager::instance()->playlist()->pause();
}

void MprisPlayer::Play() {
    StateManager::instance()->playlist()->play();
}

void MprisPlayer::Seek(qint64 us) {
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

void MprisPlayer::SetPosition(QDBusObjectPath trackId, qint64 mu) {
    if (!d->currentItem) return;
    if (trackId == this->trackPath(d->currentItem)) {
        d->currentItem->seek(mu / 1000);
    }
}

void MprisPlayer::OpenUri(QString uri) {

}

void MprisPlayer::updateCurrentItem() {
    if (d->currentItem) {
        d->currentItem->disconnect(this);
    }
    d->currentItem = StateManager::instance()->playlist()->currentItem();
    if (d->currentItem) {
        connect(d->currentItem, &MediaItem::metadataChanged, this, std::bind(&MprisPlayer::propertyChanged, this, "Metadata"));
        connect(d->currentItem, &MediaItem::elapsedChanged, this, [ = ] {
            emit Seeked(d->currentItem->elapsed() * 1000);
        });
    }
    propertyChanged("Metadata");
}


void MprisPlayer::propertyChanged(QString property) {
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

QDBusObjectPath MprisPlayer::trackPath(MediaItem* item) {
    return QDBusObjectPath(QStringLiteral("/org/thesuite/thebeat/") + QCryptographicHash::hash((item->title() + item->authors().join(",")).toUtf8(), QCryptographicHash::Sha256).toHex());
}

