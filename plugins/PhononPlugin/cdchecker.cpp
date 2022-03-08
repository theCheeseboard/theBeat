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
#include "ui_cdchecker.h"

#include "importcdpopover.h"
#include "phononcdmediaitem.h"
#include "trackinfo.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <playlist.h>
#include <pluginmediasource.h>
#include <sourcemanager.h>
#include <statemanager.h>
#include <tlogger.h>
#include <tpopover.h>
#include <tpromise.h>

#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPainter>
#include <phonon/AudioDataOutput>
#include <phonon/MediaController>
#include <phonon/MediaObject>
#include <phonon/MediaSource>

#include <tapplication.h>

#ifdef HAVE_MUSICBRAINZ
    #include "musicbrainzreleasemodel.h"
    #include <musicbrainz5/Artist.h>
    #include <musicbrainz5/ArtistCredit.h>
    #include <musicbrainz5/Medium.h>
    #include <musicbrainz5/NameCredit.h>
    #include <musicbrainz5/Query.h>
    #include <musicbrainz5/Recording.h>
    #include <musicbrainz5/Release.h>
    #include <musicbrainz5/Track.h>
#endif

using namespace Phonon;

struct CdCheckerPrivate {
        QDBusObjectPath blockPath;
        QString blockDevice;
        QDBusObjectPath cdDrivePath;

        PluginMediaSource* source;
        QStringList mbDiscIds;
        QString albumName;
        QList<TrackInfoPtr> trackInfo;

        QImage playlistBackground;
        QNetworkAccessManager mgr;

#ifdef HAVE_MUSICBRAINZ
        QString currentDiscId;
        QString currentReleaseId;
        MusicBrainz5::CReleaseList releases;
#endif
};

CdChecker::CdChecker(QDBusObjectPath blockPath, QWidget* parent) :
    AbstractLibraryBrowser(parent),
    ui(new Ui::CdChecker) {
    ui->setupUi(this);

    d = new CdCheckerPrivate();
    d->blockPath = blockPath;
    d->source = new PluginMediaSource(this);
    d->source->setName(tr("CD"));
    d->source->setIcon(QIcon::fromTheme("media-optical-audio"));

    // Get CD drive and attach to CD information
    QDBusInterface blockInterface("org.freedesktop.UDisks2", blockPath.path(), "org.freedesktop.UDisks2.Block", QDBusConnection::systemBus());
    d->cdDrivePath = blockInterface.property("Drive").value<QDBusObjectPath>();
    d->blockDevice = blockInterface.property("Device").toByteArray();
    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", d->cdDrivePath.path(), "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(checkCd()));

    ui->topWidget->installEventFilter(this);
    ui->albumSelectionSpinner->setFixedSize(SC_DPI_T(QSize(16, 16), QSize));

    ui->topWidget->setContentsMargins(0, StateManager::instance()->sources()->padTop(), 0, 0);

#ifdef HAVE_MUSICBRAINZ
    ui->releaseBox->setItemDelegate(new MusicBrainzReleaseModelDelegate(this));
#else
    ui->musicBrainzWidget->setVisible(false);
#endif

    checkCd();
}

CdChecker::~CdChecker() {
    // Deregister this source
    PhononCdMediaItem::blockDeviceGone(d->blockDevice);
    StateManager::instance()->sources()->removeSource(d->source);

    delete d;
}

AbstractLibraryBrowser::ListInformation CdChecker::currentListInformation() {
    return ListInformation();
}

void CdChecker::checkCd() {
    struct CdInformation {
            bool available = false;
            int numberOfTracks = 0;
            QStringList mbDiscId;
    };

    TPROMISE_CREATE_NEW_THREAD(CdInformation, {
        Q_UNUSED(rej)

        if (d->cdDrivePath.path() == "") {
            res(CdInformation());
            return;
        }

        CdInformation info;
        QDBusInterface cdDriveInterface("org.freedesktop.UDisks2", d->cdDrivePath.path(), "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus());
        if (cdDriveInterface.property("MediaAvailable").toBool()) {
            Qt::LayoutDirection oldDirection = tApplication::layoutDirection();

            QEventLoop* eventLoop = new QEventLoop();
            MediaObject* cdFinder = new MediaObject(this);
            MediaController* cdController = new MediaController(cdFinder);

            AudioDataOutput dummyOutput;
            createPath(cdFinder, &dummyOutput);
            connect(cdController, &MediaController::availableTitlesChanged, eventLoop, [=, &info](int tracks) {
                // New CD inserted
                info.numberOfTracks = tracks;

                if (tracks > 0) {
                    connect(cdFinder, &MediaObject::metaDataChanged, this, [=, &info]() {
                        info.mbDiscId = cdFinder->metaData(Phonon::MusicBrainzDiscIdMetaData);
                        eventLoop->quit();
                    });
                    cdController->setCurrentTitle(1);
                } else {
                    eventLoop->quit();
                }
            });

            cdFinder->setCurrentSource(MediaSource(Phonon::Cd, d->blockDevice));
            cdFinder->play();
            cdFinder->pause();

            tApplication::setLayoutDirection(oldDirection);

            // Time out after 30 seconds
            QTimer::singleShot(30000, eventLoop, &QEventLoop::quit);
            eventLoop->exec();

            eventLoop->deleteLater();
            cdFinder->deleteLater();
            cdController->deleteLater();
            info.available = true;
        }
        res(info);
    })->then([=](CdInformation info) {
        if (info.numberOfTracks == 0) {
            // No CD
            PhononCdMediaItem::blockDeviceGone(d->blockDevice);
            StateManager::instance()->sources()->removeSource(d->source);

            d->playlistBackground = QImage();
            ui->topWidget->update();
        } else {
            d->mbDiscIds = info.mbDiscId;
            d->trackInfo.clear();

#ifdef HAVE_MUSICBRAINZ
            d->releases = MusicBrainz5::CReleaseList();
            d->currentReleaseId = "";
            d->currentDiscId = "";
#endif

            for (int i = 0; i < info.numberOfTracks; i++) {
                d->trackInfo.append(TrackInfoPtr(new TrackInfo(i)));
            }

            d->source->setName(tr("CD"));
            ui->albumTitleLabel->setText(tr("CD"));
            d->albumName = tr("CD");
            StateManager::instance()->sources()->addSource(d->source);

            updateTrackListing();

            if (!info.mbDiscId.isEmpty()) {
                loadMusicbrainzData(info.mbDiscId.first());
            }
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
    // Load information from MusicBrainz
#ifdef HAVE_MUSICBRAINZ
    d->currentDiscId = discId;
    ui->musicBrainzWidget->setVisible(true);
    ui->musicBrainzStack->setCurrentWidget(ui->queryingPage);

    ui->releaseBox->clear();

    TPROMISE_CREATE_NEW_THREAD(MusicBrainz5::CReleaseList, {
        MusicBrainz5::CQuery query("thebeat-3.0");
        try {
            res(query.LookupDiscID(discId.toStdString()));
        } catch (...) {
            rej("Error");
        }
    })->then([=](MusicBrainz5::CReleaseList releases) {
          d->releases = releases;
          if (d->releases.Count() > 0) {
              tDebug("CdChecker") << "MusicBrainz lookup for " << discId << " succeded";

              if (d->releases.Count() > 1) {
                  // Populate releases
                  ui->musicBrainzStack->setCurrentWidget(ui->multipleFoundPage);
                  ui->releaseBox->setModel(new MusicBrainzReleaseModel(d->releases));
              } else {
                  ui->musicBrainzWidget->setVisible(false);
              }

              selectMusicbrainzRelease(QString::fromStdString(d->releases.Item(0)->ID()));
          } else {
              tDebug("CdChecker") << "MusicBrainz lookup for " << discId << " succeded with no results";
          }
      })->error([=](QString error) {
        tDebug("CdChecker") << "MusicBrainz lookup for " << discId << " failed";
        ui->musicBrainzStack->setCurrentWidget(ui->notFoundPage);
    });
#endif
}

void CdChecker::selectMusicbrainzRelease(QString release) {
#ifdef HAVE_MUSICBRAINZ
    d->currentReleaseId = release;

    d->playlistBackground = QImage();
    ui->topWidget->update();

    ui->albumSelectionSpinner->setVisible(true);

    tPromise<MusicBrainz5::CRelease*>::runOnNewThread([=](tPromiseFunctions<MusicBrainz5::CRelease*>::SuccessFunction res, tPromiseFunctions<MusicBrainz5::CMetadata>::FailureFunction rej) {
        try {
            MusicBrainz5::CQuery query("thebeat-3.0");
            //            res(query.LookupRelease(release.toStdString()).Clone());
            MusicBrainz5::CQuery::tParamMap params;
            params["inc"] = "artists labels recordings release-groups url-rels discids artist-credits";
            MusicBrainz5::CMetadata fullData = query.Query("release", release.toStdString(), "", params);
            if (fullData.Release()) {
                res(new MusicBrainz5::CRelease(*fullData.Release()));
            } else {
                rej("No data");
            }
        } catch (...) {
            rej("Failure");
        }
    })->then([=](MusicBrainz5::CRelease* releaseInfo) {
        // Make sure the user hasn't changed releases
        if (d->currentReleaseId != release) return;

        d->albumName = QString::fromStdString(releaseInfo->Title());
        ui->albumTitleLabel->setText(d->albumName);
        d->source->setName(d->albumName);

        // Attempt to get album art for this release
        QNetworkRequest req(QUrl("https://coverartarchive.org/release/" + QString::fromStdString(releaseInfo->ID()) + "/front"));
        req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
        QNetworkReply* artReply = d->mgr.get(req);
        connect(artReply, &QNetworkReply::finished, this, [=] {
            // Make sure the user hasn't changed releases
            if (d->currentReleaseId != release) return;

            if (artReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
                d->playlistBackground = QImage::fromData(artReply->readAll());
                ui->topWidget->update();

                for (TrackInfoPtr trackInfo : d->trackInfo) {
                    trackInfo->setAlbumArt(d->playlistBackground);
                }
            }

            ui->albumSelectionSpinner->setVisible(false);
        });

        MusicBrainz5::CMediumList* mediumList = releaseInfo->MediumList();
        for (int h = 0; h < mediumList->NumItems(); h++) {
            MusicBrainz5::CMedium* medium = mediumList->Item(h);
            if (medium->ContainsDiscID(d->currentDiscId.toStdString())) {
                MusicBrainz5::CTrackList* tracks = medium->TrackList();
                for (int i = 0; i < tracks->Count(); i++) {
                    if (d->trackInfo.count() <= i) continue;

                    MusicBrainz5::CTrack* track = tracks->Item(i);
                    MusicBrainz5::CRecording* recording = track->Recording();
                    if (recording) {
                        QStringList artists;
                        MusicBrainz5::CNameCreditList* nameCreditList = recording->ArtistCredit()->NameCreditList();
                        for (int j = 0; j < nameCreditList->NumItems(); j++) {
                            MusicBrainz5::CNameCredit* credit = nameCreditList->Item(j);
                            artists.append(QString::fromStdString(credit->Artist()->Name()));
                        }
                        artists.removeDuplicates();

                        TrackInfoPtr trackInfo = d->trackInfo.at(i);
                        trackInfo->setData(QString::fromStdString(recording->Title()), artists, QString::fromStdString(releaseInfo->Title()));
                    }
                }

                break;
            }
        }

        updateTrackListing();
        delete releaseInfo;

        if (artReply->isFinished()) ui->albumSelectionSpinner->setVisible(false);
    });
#endif
}

void CdChecker::on_tracksWidget_itemActivated(QListWidgetItem* item) {
    int track = item->data(Qt::UserRole).toInt();
    StateManager::instance()->playlist()->addItem(new PhononCdMediaItem(d->blockDevice, d->trackInfo.at(track)));
}

void CdChecker::on_enqueueAllButton_clicked() {
    for (TrackInfoPtr trackInfo : d->trackInfo) {
        StateManager::instance()->playlist()->addItem(new PhononCdMediaItem(d->blockDevice, trackInfo));
    }
}

void CdChecker::on_ejectButton_clicked() {
    // Remove any tracks in the playlist
    PhononCdMediaItem::blockDeviceGone(d->blockDevice);

    // Eject the CD
    QDBusInterface cdDriveInterface("org.freedesktop.UDisks2", d->cdDrivePath.path(), "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus());
    cdDriveInterface.call(QDBus::NoBlock, "Eject", QVariantMap());
}

void CdChecker::on_importCdButton_clicked() {
    ImportCdPopover* jp = new ImportCdPopover(d->blockDevice, d->albumName, d->trackInfo);
    tPopover* popover = new tPopover(jp);
    popover->setPopoverWidth(SC_DPI(-200));
    popover->setPopoverSide(tPopover::Bottom);
    connect(jp, &ImportCdPopover::done, popover, &tPopover::dismiss);
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    connect(popover, &tPopover::dismissed, jp, &ImportCdPopover::deleteLater);
    popover->show(this->window());
}

bool CdChecker::eventFilter(QObject* watched, QEvent* event) {
    if (watched == ui->topWidget && event->type() == QEvent::Paint) {
        QPainter painter(ui->topWidget);

        QColor backgroundCol = this->palette().color(QPalette::Window);
        if ((backgroundCol.red() + backgroundCol.green() + backgroundCol.blue()) / 3 < 127) {
            backgroundCol = QColor(0, 0, 0, 150);
        } else {
            backgroundCol = QColor(255, 255, 255, 150);
        }

        if (d->playlistBackground.isNull()) {
            //            QSvgRenderer renderer(QString(":/icons/coverimage.svg"));

            //            QRect rect;
            //            rect.setSize(renderer.defaultSize().scaled(ui->mediaLibraryInfoWidget->width(), ui->mediaLibraryInfoWidget->height(), Qt::KeepAspectRatioByExpanding));
            //            rect.setLeft(ui->mediaLibraryInfoWidget->width() / 2 - rect.width() / 2);
            //            rect.setTop(ui->mediaLibraryInfoWidget->height() / 2 - rect.height() / 2);

            //            renderer.render(&painter, rect);

            //            painter.setBrush(backgroundCol);
            //            painter.setPen(Qt::transparent);
            //            painter.drawRect(0, 0, ui->mediaLibraryInfoWidget->width(), ui->mediaLibraryInfoWidget->height());
            ui->buttonWidget->setContentsMargins(0, 0, 0, 0);
        } else {
            QRect rect;
            rect.setSize(d->playlistBackground.size().scaled(ui->topWidget->width(), ui->topWidget->height(), Qt::KeepAspectRatioByExpanding));
            rect.moveLeft(ui->topWidget->width() / 2 - rect.width() / 2);
            rect.moveTop(ui->topWidget->height() / 2 - rect.height() / 2);

            // Blur the background
            int radius = 30;
            QGraphicsBlurEffect* blur = new QGraphicsBlurEffect;
            blur->setBlurRadius(radius);

            QGraphicsScene scene;
            QGraphicsPixmapItem item;
            item.setPixmap(QPixmap::fromImage(d->playlistBackground));
            item.setGraphicsEffect(blur);
            scene.addItem(&item);

            // scene.render(&painter, QRectF(), QRectF(-radius, -radius, image.width() + radius, image.height() + radius));
            scene.render(&painter, rect.adjusted(-radius, -radius, radius, radius), QRectF(-radius, -radius, d->playlistBackground.width() + radius, d->playlistBackground.height() + radius));

            painter.setBrush(backgroundCol);
            painter.setPen(Qt::transparent);
            painter.drawRect(0, 0, ui->topWidget->width(), ui->topWidget->height());

            QRect rightRect;
            rightRect.setSize(d->playlistBackground.size().scaled(0, ui->topWidget->height() - StateManager::instance()->sources()->padTop(), Qt::KeepAspectRatioByExpanding));
            rightRect.moveRight(ui->topWidget->width());
            rightRect.moveTop(StateManager::instance()->sources()->padTop() + (ui->topWidget->height() - StateManager::instance()->sources()->padTop()) / 2 - rightRect.height() / 2);
            painter.drawImage(rightRect, d->playlistBackground.scaled(rightRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

            ui->buttonWidget->setContentsMargins(0, 0, rightRect.width(), 0);
        }
    }
    return false;
}

void CdChecker::on_musicBrainzStack_currentChanged(int arg1) {
    ui->musicBrainzStack->setFixedHeight(ui->musicBrainzStack->widget(arg1)->heightForWidth(ui->musicBrainzStack->width()));
}

void CdChecker::resizeEvent(QResizeEvent* event) {
    ui->musicBrainzStack->setFixedHeight(ui->musicBrainzStack->widget(ui->musicBrainzStack->currentIndex())->heightForWidth(ui->musicBrainzStack->width()));
}

void CdChecker::on_releaseBox_currentIndexChanged(int index) {
    selectMusicbrainzRelease(ui->releaseBox->itemData(index).toString());
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
