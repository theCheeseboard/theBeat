#include "cdchecker.h"
#include "ui_cdchecker.h"

#include <statemanager.h>
#include <sourcemanager.h>
#include <playlist.h>
#include "wincdmediaitem.h"
#include <pluginmediasource.h>
#include "trackinfo.h"
#include <tlogger.h>
#include <winrt/CDLib.h>
#include "audiocdplayerthread.h"
#include <winrt/Windows.Foundation.Collections.h>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

struct CdCheckerPrivate {
    PluginMediaSource* source;
    winrt::CDLib::IAudioCDDrive drive;
    QList<TrackInfoPtr> trackInfo;

    QImage playlistBackground;
    QNetworkAccessManager mgr;

    QTimer* metadataTimer;
};

CdChecker::CdChecker(QChar driveLetter, QWidget* parent) :
    AbstractLibraryBrowser(parent),
    ui(new Ui::CdChecker) {
    ui->setupUi(this);
    d = new CdCheckerPrivate();
    d->source = new PluginMediaSource(this);

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

    d->source->setName(tr("CD"));
    d->source->setIcon(QIcon::fromTheme("media-optical-audio"));

    d->drive.MediaChanged([ = ](winrt::CDLib::IAudioCDDrive drive) {
        Q_UNUSED(drive);
        checkCd();
    });
    checkCd();

    connect(StateManager::instance()->sources(), &SourceManager::padTopChanged, this, [this](int padTop) {
        ui->topWidget->setContentsMargins(0, padTop, 0, 0);
    });
    ui->topWidget->setContentsMargins(0, StateManager::instance()->sources()->padTop(), 0, 0);
    ui->importCdButton->setVisible(false);

//    try {
//        tDebug("CdChecker") << "Playing track 1";
//        auto insertedMedia = d->drive.InsertedMedia();
//        tDebug("CdChecker") << "Got inserted media";
//        auto track = insertedMedia.Tracks().GetAt(0);
//        tDebug("CdChecker") << "Got track " << QString::fromUtf16((ushort*) track.Name().c_str());
//        audioCdPlayer.PlayTrack(track);
//    } catch (winrt::hresult_error e) {
//        tDebug("CdChecker") << QString::number(e.code(), 16);
//        tDebug("CdChecker") << QString::fromUtf16((ushort*) e.message().c_str());
//    }
}

CdChecker::~CdChecker() {
    delete d;
    delete ui;
}

AbstractLibraryBrowser::ListInformation CdChecker::currentListInformation()
{
    return ListInformation();
}

void CdChecker::checkCd() {
    auto media = d->drive.InsertedMedia();
    if (media && media.Tracks().Size() > 0) {
        StateManager::instance()->sources()->addSource(d->source);

        d->trackInfo.clear();
        for (uint i = 0; i < media.Tracks().Size(); i++) {
            TrackInfoPtr trackInfo = TrackInfoPtr(new TrackInfo(i));
            d->trackInfo.append(trackInfo);
        }

        d->metadataTimer->start();
        getMetadata();

//        QString albumArt = QString::fromUtf16(reinterpret_cast<const ushort*>(firstTrack.AlbumCoverUrl().c_str()));
//        tDebug("CdChecker") << albumArt;

//        //Attempt to get album art for this release
//        QNetworkRequest req((QUrl(albumArt)));
//        req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
//        QNetworkReply* artReply = d->mgr.get(req);
//        connect(artReply, &QNetworkReply::finished, this, [ = ] {
//            //TODO: Make sure the user hasn't changed releases
////            if (d->currentReleaseId != release) return;

//            if (artReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
//                d->playlistBackground = QImage::fromData(artReply->readAll());
//                ui->topWidget->update();

//                for (TrackInfoPtr trackInfo : d->trackInfo) {
//                    trackInfo->setAlbumArt(d->playlistBackground);
//                }
//            }

//            ui->topWidget->update();
//        });

        updateTrackListing();
    } else {
        WinCdMediaItem::driveGone(d->drive.DriveLetter().Value());
        StateManager::instance()->sources()->removeSource(d->source);

        d->playlistBackground = QImage();
        ui->topWidget->update();

        d->source->setName(tr("CD"));
        ui->albumTitleLabel->setText(tr("CD"));

        d->metadataTimer->stop();
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
    QString album = QString::fromUtf16(reinterpret_cast<const char16_t*>(firstTrack.AlbumTitle().c_str()));
    d->source->setName(album);
    ui->albumTitleLabel->setText(album);

    updateTrackListing();
}

void CdChecker::updateTrackListing() {
    if (ui->tracksWidget->count() == d->trackInfo.count()) {
        for (TrackInfoPtr info : d->trackInfo) {
            QListWidgetItem* item = ui->tracksWidget->item(info->track());
            item->setText(info->title());
        }
    } else {
        ui->tracksWidget->clear();
        for (TrackInfoPtr info : d->trackInfo) {
            QListWidgetItem* item = new QListWidgetItem();
            item->setText(info->title());
            item->setData(Qt::UserRole, info->track());
            ui->tracksWidget->addItem(item);
        }
    }
}

void CdChecker::on_ejectButton_clicked() {
    d->drive.Eject();
}

void CdChecker::on_tracksWidget_itemActivated(QListWidgetItem* item) {
    winrt::CDLib::IAudioCDTrack track = d->drive.InsertedMedia().Tracks().GetAt(item->data(Qt::UserRole).toInt());
    StateManager::instance()->playlist()->addItem(new WinCdMediaItem(d->drive.DriveLetter().Value(), track));
}

void CdChecker::on_enqueueAllButton_clicked() {
    for (TrackInfoPtr trackInfo : d->trackInfo) {
        winrt::CDLib::IAudioCDTrack track = d->drive.InsertedMedia().Tracks().GetAt(trackInfo->track());
        StateManager::instance()->playlist()->addItem(new WinCdMediaItem(d->drive.DriveLetter().Value(), track));
    }
}

void CdChecker::on_playAllButton_clicked() {
    StateManager::instance()->playlist()->clear();
    ui->enqueueAllButton->click();
}

void CdChecker::on_shuffleAllButton_clicked() {
    ui->enqueueAllButton->click();
    StateManager::instance()->playlist()->setShuffle(true);
    StateManager::instance()->playlist()->next();
}
