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

struct CdCheckerPrivate {
    PluginMediaSource* source;
    winrt::CDLib::IAudioCDDrive drive;
    QList<TrackInfoPtr> trackInfo;
};

CdChecker::CdChecker(QChar driveLetter, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::CdChecker) {
    ui->setupUi(this);
    d = new CdCheckerPrivate();
    d->source = new PluginMediaSource(this);

    winrt::CDLib::IAudioCDPlayer audioCdPlayer = AudioCdPlayerThread::instance()->player();
    auto drives = audioCdPlayer.GetDrives();

    for (uint i = 0; i < drives.Size(); i++) {
        winrt::CDLib::IAudioCDDrive drive = drives.GetAt(i);
        auto letter = drive.DriveLetter();
        if (letter.Value() == driveLetter) {
            d->drive = drive;
        }
    }

    d->drive.MediaChanged([ = ](winrt::CDLib::IAudioCDDrive drive) {
        Q_UNUSED(drive);
        checkCd();
    });
    checkCd();

    d->source->setName(tr("CD"));
    d->source->setIcon(QIcon::fromTheme("media-optical-audio"));

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

void CdChecker::checkCd() {
    auto media = d->drive.InsertedMedia();
    if (media && media.Tracks().Size() > 0) {
        StateManager::instance()->sources()->addSource(d->source);

        d->trackInfo.clear();
        for (uint i = 0; i < media.Tracks().Size(); i++) {
            d->trackInfo.append(TrackInfoPtr(new TrackInfo(i)));
        }

        updateTrackListing();
    } else {
        WinCdMediaItem::driveGone(d->drive.DriveLetter().Value());
        StateManager::instance()->sources()->removeSource(d->source);
    }
}

void CdChecker::updateTrackListing() {
    ui->tracksWidget->clear();
    for (TrackInfoPtr info : d->trackInfo) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(info->title());
        item->setData(Qt::UserRole, info->track());
        ui->tracksWidget->addItem(item);
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
