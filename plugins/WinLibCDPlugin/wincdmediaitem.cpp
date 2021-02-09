#include "wincdmediaitem.h"

#include <QImage>
#include <QVariant>
#include <QMediaMetaData>
#include <QPointer>
#include <QTimer>
#include <statemanager.h>
#include <playlist.h>
#include <tlogger.h>
#include "audiocdplayerthread.h"

struct WinCdMediaItemPrivate {
    QChar driveLetter;
    winrt::CDLib::IAudioCDTrack track;
    QTimer* timer;

    winrt::event_token finishedPlayingToken;

    static QPointer<WinCdMediaItem> currentItem;
    static QMultiMap<QChar, WinCdMediaItem*> driveMapping;
};

QPointer<WinCdMediaItem> WinCdMediaItemPrivate::currentItem = QPointer<WinCdMediaItem>();
QMultiMap<QChar, WinCdMediaItem*> WinCdMediaItemPrivate::driveMapping = QMultiMap<QChar, WinCdMediaItem*>();

WinCdMediaItem::WinCdMediaItem(QChar driveLetter, winrt::CDLib::IAudioCDTrack track) : MediaItem() {
    d = new WinCdMediaItemPrivate();
    d->track = track;

    d->driveLetter = driveLetter;
    d->driveMapping.insertMulti(driveLetter, this);

    d->timer = new QTimer(this);
    d->timer->setInterval(100);
    connect(d->timer, &QTimer::timeout, this, [ = ] {
        emit durationChanged();
        emit elapsedChanged();
    });

    d->finishedPlayingToken = AudioCdPlayerThread::instance()->player().FinishedPlayingTrack([ = ] {
        if (d->currentItem == this) {
            QTimer::singleShot(0, this, &WinCdMediaItem::done);
        }
    });
}

WinCdMediaItem::~WinCdMediaItem() {
    d->driveMapping.remove(d->driveLetter, this);
    AudioCdPlayerThread::instance()->player().FinishedPlayingTrack(d->finishedPlayingToken);
    delete d;
}

void WinCdMediaItem::driveGone(QChar driveLetter) {
    QList<WinCdMediaItem*> items = WinCdMediaItemPrivate::driveMapping.values(driveLetter);
    for (WinCdMediaItem* item : items) {
        StateManager::instance()->playlist()->removeItem(item);
        item->deleteLater();
    }
    WinCdMediaItemPrivate::driveMapping.remove(driveLetter);
}

void WinCdMediaItem::play() {
    if (d->currentItem == this) {
        AudioCdPlayerThread::instance()->player().Resume();
    } else {
        AudioCdPlayerThread::instance()->player().PlayTrack(d->track);
        d->currentItem = this;
    }

    d->timer->start();
}

void WinCdMediaItem::pause() {
    if (d->currentItem == this) {
        AudioCdPlayerThread::instance()->player().Pause();
        d->timer->stop();
    }
}

void WinCdMediaItem::stop() {
    if (d->currentItem == this) {
        AudioCdPlayerThread::instance()->player().Pause();
        d->timer->stop();
    }
}

void WinCdMediaItem::seek(quint64 ms) {
    if (d->currentItem == this) AudioCdPlayerThread::instance()->player().Seek(std::chrono::duration_cast<winrt::Windows::Foundation::TimeSpan>(std::chrono::duration<quint64, std::milli>(ms)));
}

quint64 WinCdMediaItem::elapsed() {
    return std::chrono::duration_cast<std::chrono::duration<quint64, std::milli>>(AudioCdPlayerThread::instance()->player().CurrentPosition()).count();
}

quint64 WinCdMediaItem::duration() {
    return std::chrono::duration_cast<std::chrono::duration<quint64, std::milli>>(d->track.Duration()).count();
}

QString WinCdMediaItem::title() {
    return QString::fromUtf16(reinterpret_cast<const ushort*>(d->track.Name().c_str()));
}

QStringList WinCdMediaItem::authors() {
    return {};
}

QString WinCdMediaItem::album() {
    return "CD";
}

QImage WinCdMediaItem::albumArt() {
    return QImage();
}

QVariant WinCdMediaItem::metadata(QString key) {
    if (key == QMediaMetaData::TrackNumber) {
        return d->track.TrackNumber();
    }
    return QVariant();
}
