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

#ifdef HAVE_MUSICBRAINZ
    #include "../../PhononPlugin/musicbrainzreleasemodel.h"
    #include <musicbrainz5/Artist.h>
    #include <musicbrainz5/ArtistCredit.h>
    #include <musicbrainz5/Medium.h>
    #include <musicbrainz5/NameCredit.h>
    #include <musicbrainz5/Query.h>
    #include <musicbrainz5/Recording.h>
    #include <musicbrainz5/Release.h>
    #include <musicbrainz5/Track.h>
#endif

using namespace Qt::Literals;

CdChecker::CdChecker(QString directory, QObject* parent) :
    QAbstractListModel(parent) {
    d = new CdCheckerPrivate();
    d->albumName = tr("CD");
    d->directory = directory;
    d->source = new MacCdPluginMediaSource(this);
    d->source->setIcon(QIcon::fromTheme("media-optical-audio"));

    // #ifdef HAVE_MUSICBRAINZ
    //     ui->releaseBox->setItemDelegate(new MusicBrainzReleaseModelDelegate(this));
    // #else
    //     ui->musicBrainzWidget->setVisible(false);
    // #endif

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
            QString mbDiscId;
    };

    auto info = co_await QtConcurrent::run([this](QString directory) {
        CdInformation info;
        QDir dir(directory);

        info.available = true;
        info.numberOfTracks = dir.entryList(QDir::Files).count();

        info.mbDiscId = calculateMbDiscId();

        return info;
    }, d->directory);

    if (info.numberOfTracks == 0) {
        // No CD
        StateManager::instance()->sources()->removeSource(d->source);

        d->playlistBackground = QImage();
    } else {
        this->beginResetModel();
        d->mbDiscId = info.mbDiscId;
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
        d->albumName = tr("CD");
        StateManager::instance()->sources()->addSource(d->source);
        emit albumNameChanged();
        this->endResetModel();

        if (!info.mbDiscId.isEmpty()) {
            loadMusicbrainzData(info.mbDiscId);
        }
    }
}

QCoro::Task<> CdChecker::loadMusicbrainzData(QString discId) {
    // Load information from MusicBrainz
#ifdef HAVE_MUSICBRAINZ
    d->currentDiscId = discId;

    QPointer<QObject> context = this;

    auto releases = co_await QtConcurrent::run([](QString discId) {
        MusicBrainz5::CQuery query("thebeat-3.0");
        try {
            return query.LookupDiscID(discId.toStdString());
        } catch (...) {
            return MusicBrainz5::CReleaseList{};
        }
    }, discId);

    if (!context) co_return;
    d->releases = releases;
    if (d->releases.Count() > 0) {
        tDebug("CdChecker") << "MusicBrainz lookup for " << discId << " succeded";

        if (d->releases.Count() > 1) {
            // Populate releases
            // ui->musicBrainzStack->setCurrentWidget(ui->multipleFoundPage);
            // ui->releaseBox->setModel(new MusicBrainzReleaseModel(d->releases));
        } else {
            // ui->musicBrainzWidget->setVisible(false);
        }

        selectMusicbrainzRelease(QString::fromStdString(d->releases.Item(0)->ID()));
    } else {
        tDebug("CdChecker") << "MusicBrainz lookup for " << discId << " succeded with no results";
    }
#endif
}

QCoro::Task<> CdChecker::selectMusicbrainzRelease(QString release) {
#ifdef HAVE_MUSICBRAINZ
    d->currentReleaseId = release;
    d->playlistBackground = QImage();

    auto releaseInfo = co_await QtConcurrent::run([](QString release) -> MusicBrainz5::CRelease* {
        MusicBrainz5::CQuery query("thebeat-3.0");
        MusicBrainz5::CQuery::tParamMap params;
        params["inc"] = "artists labels recordings release-groups url-rels discids artist-credits";
        MusicBrainz5::CMetadata fullData = query.Query("release", release.toStdString(), "", params);
        if (fullData.Release()) {
            return new MusicBrainz5::CRelease(*fullData.Release());
        } else {
            return nullptr;
        }
    }, release);

    // Make sure the user hasn't changed releases
    if (d->currentReleaseId != release) co_return;

    d->albumName = QString::fromStdString(releaseInfo->Title());
    emit albumNameChanged();

    // Attempt to get album art for this release
    auto releaseId = QString::fromStdString(releaseInfo->ID());
    QNetworkRequest req(QUrl(u"https://coverartarchive.org/release/%1/front"_s.arg(releaseId)));
    QNetworkReply* artReply = d->mgr.get(req);
    connect(artReply, &QNetworkReply::finished, this, [this, release, artReply] {
        // Make sure the user hasn't changed releases
        if (d->currentReleaseId != release) return;

        if (artReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
            d->playlistBackground = QImage::fromData(artReply->readAll());

            for (TrackInfoPtr trackInfo : d->trackInfo) {
                trackInfo->setAlbumArt(d->playlistBackground);
            }
            emit dataChanged(index(0), index(rowCount() - 1));
        }
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
                if (!recording) continue;

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

            break;
        }
    }

    emit dataChanged(index(0), index(rowCount() - 1));
    delete releaseInfo;
#endif
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
