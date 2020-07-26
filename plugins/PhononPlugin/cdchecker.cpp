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
#include "cdchecker.h"

#include <ui_cdchecker.h>
#include <QDBusInterface>
#include <QDBusConnection>
#include <pluginmediasource.h>
#include <statemanager.h>
#include <sourcemanager.h>
#include <playlist.h>
#include <tpromise.h>
#include "phononcdmediaitem.h"
#include "trackinfo.h"

#include <phonon/MediaObject>
#include <phonon/MediaController>
#include <phonon/MediaSource>
#include <phonon/AudioDataOutput>

using namespace Phonon;

struct CdCheckerPrivate {
    QString blockDevice;
    QDBusObjectPath cdDrivePath;

    PluginMediaSource* source;
    QStringList mbDiscIds;
    QList<TrackInfoPtr> trackInfo;
};

CdChecker::CdChecker(QString blockDevice, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::CdChecker) {
    ui->setupUi(this);

    d = new CdCheckerPrivate();
    d->blockDevice = blockDevice;
    d->source = new PluginMediaSource(this);
    d->source->setName(tr("CD"));
    d->source->setIcon(QIcon::fromTheme("media-optical-audio"));

    //Get CD drive and attach to CD information
    QDBusInterface blockInterface("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2/block_devices/" + blockDevice, "org.freedesktop.UDisks2.Block", QDBusConnection::systemBus());
    d->cdDrivePath = blockInterface.property("Drive").value<QDBusObjectPath>();
    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", d->cdDrivePath.path(), "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(checkCd()));

    checkCd();
}

CdChecker::~CdChecker() {
    delete d;
}

void CdChecker::checkCd() {
    struct CdInformation {
        bool available = false;
        int numberOfTracks = 0;
        QStringList mbDiscId;
    };

    tPromise<CdInformation>::runOnNewThread([ = ](tPromiseFunctions<CdInformation>::SuccessFunction res, tPromiseFunctions<CdInformation>::FailureFunction rej) {
        if (d->cdDrivePath.path() == "") {
            res(CdInformation());
            return;
        }

        CdInformation info;
        QDBusInterface cdDriveInterface("org.freedesktop.UDisks2", d->cdDrivePath.path(), "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus());
        if (cdDriveInterface.property("MediaAvailable").toBool()) {

            QEventLoop* eventLoop = new QEventLoop();
            MediaObject* cdFinder = new MediaObject();
            MediaController* cdController = new MediaController(cdFinder);
            AudioDataOutput dummyOutput;
            createPath(cdFinder, &dummyOutput);
            connect(cdController, &MediaController::availableTitlesChanged, [ =, &info](int tracks) {
                //New CD inserted
                info.numberOfTracks = tracks;

                if (tracks > 0) {
                    connect(cdFinder, &MediaObject::metaDataChanged, [ =, &info]() {
                        info.mbDiscId = cdFinder->metaData(Phonon::MusicBrainzDiscIdMetaData);
                        eventLoop->quit();
                    });
                    cdController->setCurrentTitle(1);
                } else {
                    eventLoop->quit();
                }
            });

            cdFinder->setCurrentSource(MediaSource(Phonon::Cd, "/dev/" + d->blockDevice));
            cdFinder->play();
            cdFinder->pause();

            eventLoop->exec();

            eventLoop->deleteLater();
            cdFinder->deleteLater();
            cdController->deleteLater();
            info.available = true;
        }
        res(info);
    })->then([ = ](CdInformation info) {
        if (info.numberOfTracks == 0) {
            //No CD
            PhononCdMediaItem::blockDeviceGone(d->blockDevice);
            StateManager::instance()->sources()->removeSource(d->source);
        } else {
            d->mbDiscIds = info.mbDiscId;
            d->trackInfo.clear();

            for (int i = 0; i < info.numberOfTracks; i++) {
                d->trackInfo.append(TrackInfoPtr(new TrackInfo(i)));
            }

            d->source->setName(tr("CD"));
            StateManager::instance()->sources()->addSource(d->source);

            updateTrackListing();
        }
    });
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

void CdChecker::loadMusicbrainzData(QString discId) {

}


void CdChecker::on_tracksWidget_itemActivated(QListWidgetItem* item) {
    int track = item->data(Qt::UserRole).toInt();
    StateManager::instance()->playlist()->addItem(new PhononCdMediaItem(d->blockDevice, d->trackInfo.at(track)));
}
