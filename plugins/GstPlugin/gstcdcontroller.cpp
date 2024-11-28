#include "gstcdcontroller.h"

#include "gsttrackinfo.h"
#include "mediaitem/gstcdpluginmediasource.h"
#include <DriveObjects/blockinterface.h>
#include <DriveObjects/diskobject.h>
#include <DriveObjects/driveinterface.h>
#include <mediaitem/gstcdplayback.h>
#include <pluginmediasource.h>
#include <sourcemanager.h>
#include <statemanager.h>

#ifdef HAVE_CDIO
    #include <cdio++/cdio.hpp>
#endif

struct GstCdControllerPrivate {
        PluginMediaSource* source;
        DiskObject* disk;

        QString albumName;
        QList<GstTrackInfoPtr> trackInfo;
};

GstCdController::GstCdController(DiskObject* disk, QWidget* parent) :
    QAbstractListModel(parent) {
    d = new GstCdControllerPrivate();
    d->disk = disk;

    d->albumName = tr("CD");

    d->source = new GstCdPluginMediaSource(this);
    d->source->setIcon(QIcon::fromTheme("media-optical-audio"));

    auto drive = d->disk->interface<BlockInterface>()->drive();
    for (auto i = 0; i < drive->audioTracks(); i++) {
        d->trackInfo.append(GstTrackInfoPtr(new GstTrackInfo(i)));
    }

    readCd();

    StateManager::instance()->sources()->addSource(d->source);

    updateTracks();
}

GstCdController::~GstCdController() {
    StateManager::instance()->sources()->removeSource(d->source);
    delete d;
}

QString GstCdController::albumName() {
    return d->albumName;
}

QCoro::Task<> GstCdController::eject() {
    auto drive = d->disk->interface<BlockInterface>()->drive();
    co_await drive->eject();
}

MediaItem* GstCdController::mediaItem(int row) {
    return new GstCdPlayback(d->disk->interface<BlockInterface>()->blockName(), row + 1, d->trackInfo.at(row));
}

void GstCdController::readCd() {
#ifdef HAVE_CDIO
    CdioDevice device;
    if (!device.open(d->disk->interface<BlockInterface>()->blockName().toUtf8().constData(), DRIVER_DEVICE)) return;

    auto cdText = device.getCdtext();
    QMap<QString, QString> discFields;
    for (auto i = static_cast<cdtext_field_t>(0); i < MAX_CDTEXT_FIELDS; i++) {
        auto field = cdText->getConst(i, 0);
        if (field) {
            auto fieldName = QString::fromLatin1(cdText->field2str(i));
            auto fieldValue = QString::fromLatin1(field);
            discFields.insert(fieldName, fieldValue);
        }
    }

    for (auto track = 1; track < d->trackInfo.length() + 1; track++) {
        QMap<QString, QString> trackFields;

        for (auto i = static_cast<cdtext_field_t>(0); i < MAX_CDTEXT_FIELDS; i++) {
            auto field = cdText->getConst(i, track);
            if (field) {
                auto fieldName = QString::fromLatin1(cdText->field2str(i));
                auto fieldValue = QString::fromLatin1(field);
                trackFields.insert(fieldName, fieldValue);
            }
        }

        auto trackInfo = d->trackInfo.at(track - 1);
        if (trackFields.contains("TITLE") && discFields.contains("TITLE") && discFields.contains("PERFORMER")) {
            trackInfo->setData(trackFields.value("TITLE"), {discFields.value("PERFORMER")}, discFields.value("TITLE"));
        }
    }

    emit dataChanged(index(0), index(rowCount() - 1));

    if (discFields.contains("TITLE")) {
        d->albumName = discFields.value("TITLE");
        emit albumNameChanged();
    }
#endif
}

void GstCdController::updateTracks() {
    auto drive = d->disk->interface<BlockInterface>()->drive();
    // ui->tracksWidget->clear();
    // for (auto i = 0; i < drive->audioTracks(); i++) {
    //     QListWidgetItem* item = new QListWidgetItem();
    //     item->setText(tr("Track %1").arg(i + 1));
    //     item->setData(Qt::UserRole, i + 1);
    //     ui->tracksWidget->addItem(item);
    // }
}
// void GstCdController::on_tracksWidget_itemActivated(QListWidgetItem* item) {
//     int track = item->data(Qt::UserRole).toInt();
//     StateManager::instance()->playlist()->addItem(new GstCdPlayback(d->disk->interface<BlockInterface>()->blockName(), track));
// }

int GstCdController::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;

    return d->trackInfo.length();
}

QVariant GstCdController::data(const QModelIndex& index, int role) const {
    if (index.parent().isValid()) return {};

    auto track = d->trackInfo.at(index.row());
    switch (role) {
        case TitleRole:
            return track->title();
        case ArtistRole:
            return track->artist();
        case AlbumRole:
            return track->album();
    }

    return {};
}

QHash<int, QByteArray> GstCdController::roleNames() const {
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
