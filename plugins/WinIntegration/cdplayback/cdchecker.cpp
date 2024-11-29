#include "cdchecker.h"

#include <statemanager.h>
#include <sourcemanager.h>
#include <playlist.h>
#include "wincdmediaitem.h"
#include "wincdpluginmediasource.h"
#include "trackinfo.h"
#include <tlogger.h>
#include <winrt/CDLib.h>
#include "audiocdplayerthread.h"
#include <winrt/Windows.Foundation.Collections.h>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

struct CdCheckerPrivate {
    WinCdPluginMediaSource* source;
    winrt::CDLib::IAudioCDDrive drive;
    QList<TrackInfoPtr> trackInfo;
    QString albumName;

    QImage playlistBackground;
    QNetworkAccessManager mgr;

    QTimer* metadataTimer;
};

CdChecker::CdChecker(QChar driveLetter, QWidget* parent) :
    QAbstractListModel(parent) {
    d = new CdCheckerPrivate();
    d->source = new WinCdPluginMediaSource(this);

    d->metadataTimer = new QTimer(this);
    d->metadataTimer->setInterval(1000);
    connect(d->metadataTimer, &QTimer::timeout, this, &CdChecker::getMetadata);

    winrt::CDLib::IAudioCDPlayer audioCdPlayer = AudioCdPlayerThread::instance()->player();
    auto drives = audioCdPlayer.GetDrives();

    for (uint i = 0; i < drives.Size(); i++) {
        winrt::CDLib::IAudioCDDrive drive = drives.GetAt(i);
        auto letter = drive.DriveLetter();
        if (letter.Value() == driveLetter) {
            d->drive = drive;
        }
    }

    d->source->setIcon(QIcon::fromTheme("media-optical-audio"));

    d->drive.MediaChanged([ this ](winrt::CDLib::IAudioCDDrive drive) {
        Q_UNUSED(drive);
        checkCd();
    });
    checkCd();
}

CdChecker::~CdChecker() {
    delete d;
}

void CdChecker::checkCd() {
    auto media = d->drive.InsertedMedia();
    if (media && media.Tracks().Size() > 0) {
        this->beginResetModel();
        StateManager::instance()->sources()->addSource(d->source);

        d->trackInfo.clear();
        for (uint i = 0; i < media.Tracks().Size(); i++) {
            TrackInfoPtr trackInfo = TrackInfoPtr(new TrackInfo(i));
            d->trackInfo.append(trackInfo);
        }

        d->metadataTimer->start();
        getMetadata();

        updateTrackListing();
        this->endResetModel();
    } else {
        this->beginResetModel();
        WinCdMediaItem::driveGone(d->drive.DriveLetter().Value());
        StateManager::instance()->sources()->removeSource(d->source);

        d->playlistBackground = QImage();

        d->albumName = tr("CD");
        emit albumNameChanged();

        d->metadataTimer->stop();
        d->trackInfo.clear();
        this->endResetModel();
    }
}

void CdChecker::getMetadata() {
    auto media = d->drive.InsertedMedia();
    for (uint i = 0; i < media.Tracks().Size(); i++) {
        TrackInfoPtr trackInfo = d->trackInfo.at(i);
        winrt::CDLib::IAudioCDTrack cdTrack = media.Tracks().GetAt(i);
        trackInfo->setData(QString::fromUtf16(reinterpret_cast<const char16_t*>(cdTrack.Title().c_str())),
        {QString::fromUtf16(reinterpret_cast<const char16_t*>(cdTrack.Artist().c_str()))},
        QString::fromUtf16(reinterpret_cast<const char16_t*>(cdTrack.AlbumTitle().c_str())));
    }

    winrt::CDLib::IAudioCDTrack firstTrack = media.Tracks().GetAt(0);
    d->albumName = QString::fromUtf16(reinterpret_cast<const char16_t*>(firstTrack.AlbumTitle().c_str()));
    emit albumNameChanged();

    updateTrackListing();
}

void CdChecker::updateTrackListing() {
    emit dataChanged(index(0), index(rowCount() - 1));
}

QString CdChecker::albumName() {
    return d->albumName;
}

QCoro::Task<> CdChecker::eject() {
    d->drive.Eject();
    co_return;
}

MediaItem* CdChecker::mediaItem(int row) {
    winrt::CDLib::IAudioCDTrack track = d->drive.InsertedMedia().Tracks().GetAt(row);
    return new WinCdMediaItem(d->drive.DriveLetter().Value(), track);
}

int CdChecker::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;

    return d->trackInfo.count();
}

QVariant CdChecker::data(const QModelIndex& index, int role) const {
    if (index.parent().isValid()) return {};

    auto trackInfo = d->trackInfo.at(index.row());
    switch (role) {
        case TitleRole:
            return trackInfo->title();
        case ArtistRole:
            return trackInfo->artist();
        case AlbumRole:
            return trackInfo->album();
        case TrackRole:
            return trackInfo->track() + 1;
    }

    return {};
}

QHash<int, QByteArray> CdChecker::roleNames() const {
    return {
        {PathRole,     "path"    },
        {TitleRole,    "title"   },
        {ArtistRole,   "artist"  },
        {AlbumRole,    "album"   },
        {DurationRole, "duration"},
        {TrackRole,    "track"   },
        {AlbumArtRole, "albumArt"},
        {ErrorRole,    "error"   },
        {SortRole,     "sort"    }
    };
}
