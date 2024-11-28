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
#include "cdchecker_p.h"

#include "maccdmediaitem.h"
#include "maccdpluginmediasource.h"
#include "trackinfo.h"
#include <QCoroFuture>
#include <QCryptographicHash>
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

using namespace Qt::Literals;

CdChecker::CdChecker(QString directory, QObject* parent) :
    QAbstractListModel(parent) {
    d = new CdCheckerPrivate();
    d->albumName = tr("CD");
    d->directory = directory;
    d->source = new MacCdPluginMediaSource(this);
    d->source->setIcon(QIcon::fromTheme("media-optical-audio"));

    checkCd();
}

CdChecker::~CdChecker() {
    // Deregister this source
    MacCdMediaItem::volumeGone(d->directory);
    StateManager::instance()->sources()->removeSource(d->source);

    delete d;
}

QString CdChecker::albumName() {
    return d->albumName;
}

QCoro::Task<> CdChecker::checkCd() {
    struct CdInformation {
            bool available = false;
            int numberOfTracks = 0;
    };

    auto info = co_await QtConcurrent::run([this](QString directory) {
        CdInformation info;
        QDir dir(directory);

        info.available = true;
        info.numberOfTracks = dir.entryList(QDir::Files).count();

        return info;
    }, d->directory);

    if (info.numberOfTracks == 0) {
        // No CD
        StateManager::instance()->sources()->removeSource(d->source);

        d->playlistBackground = QImage();
    } else {
        this->beginResetModel();
        d->trackInfo.clear();

        for (int i = 0; i < info.numberOfTracks; i++) {
            d->trackInfo.append(TrackInfoPtr(new TrackInfo(i)));
        }

        d->source->setName(tr("CD"));
        d->albumName = tr("CD");
        StateManager::instance()->sources()->addSource(d->source);
        emit albumNameChanged();
        this->endResetModel();
        setupMusicBrainzClient();

        if (d->musicBrainzClient) {
            connect(d->musicBrainzClient, &MusicBrainzClient::albumNameChanged, this, [this] {
                d->albumName = d->musicBrainzClient->albumName();
                emit albumNameChanged();
            });
            connect(d->musicBrainzClient, &MusicBrainzClient::albumArtChanged, this, [this] {
                for (auto track : d->trackInfo) {
                    track->setAlbumArt(d->musicBrainzClient->albumArt());
                }
                emit dataChanged(index(0), index(rowCount() - 1));
            });
            connect(d->musicBrainzClient, &MusicBrainzClient::tracksChanged, this, [this] {
                for (auto i = 0; i < d->trackInfo.length(); i++) {
                    if (d->musicBrainzClient->trackCount() <= i) return;

                    auto track = d->trackInfo.at(i);
                    auto mbTrack = d->musicBrainzClient->track(i);
                    track->setData(mbTrack.title, mbTrack.artists, mbTrack.album);
                }
                emit dataChanged(index(0), index(rowCount() - 1));
            });
        }
    }
}

void CdChecker::on_importCdButton_clicked() {
    // TODO: Import the CD
    //    ImportCdPopover* jp = new ImportCdPopover(d->blockDevice, d->albumName, d->trackInfo);
    //    tPopover* popover = new tPopover(jp);
    //    popover->setPopoverWidth(SC_DPI(-200));
    //    popover->setPopoverSide(tPopover::Bottom);
    //    connect(jp, &ImportCdPopover::done, popover, &tPopover::dismiss);
    //    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    //    connect(popover, &tPopover::dismissed, jp, &ImportCdPopover::deleteLater);
    //    popover->show(this->window());
}

int CdChecker::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;

    return d->trackInfo.length();
}

QVariant CdChecker::data(const QModelIndex& index, int role) const {
    if (index.parent().isValid()) return {};

    auto track = d->trackInfo.at(index.row());
    switch (role) {
        case TitleRole:
            return track->title();
        case AlbumRole:
            return track->album();
        case ArtistRole:
            return track->artist();
        case TrackRole:
            return track->track() + 1;
    }

    return {};
}

MediaItem* CdChecker::mediaItem(int row) {
    return new MacCdMediaItem(d->directory, d->trackInfo.at(row));
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
