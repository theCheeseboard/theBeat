#include "paranoiacdcontroller.h"

#include "paranoiacdpluginmediasource.h"
#include "paranoiamediaitem.h"
#include "paranoiaplayer.h"
#include "paranoiatrackinfo.h"
#include <DriveObjects/blockinterface.h>
#include <DriveObjects/diskobject.h>
#include <DriveObjects/driveinterface.h>
// #include <mediaitem/paranoiacdplayback.h>
#include <pluginmediasource.h>
#include <sourcemanager.h>
#include <statemanager.h>

#include <QAudioSink>

#include <cdio++/cdio.hpp>

struct ParanoiaCdControllerPrivate {
        PluginMediaSource* source;
        DiskObject* disk;
        ParanoiaPlayer* player = nullptr;
        QAudioSink* sink = nullptr;
        QIODevice* sinkOutput = nullptr;

        QString albumName;
        QList<ParanoiaTrackInfoPtr> trackInfo;
        CdioDevice device;
};

ParanoiaCdController::ParanoiaCdController(DiskObject* disk, QWidget* parent) :
    QAbstractListModel(parent) {
    d = new ParanoiaCdControllerPrivate();
    d->disk = disk;

    d->albumName = tr("CD");

    d->source = new ParanoiaCdPluginMediaSource(this);
    d->source->setIcon(QIcon::fromTheme("media-optical-audio"));

    auto drive = d->disk->interface<BlockInterface>()->drive();
    for (auto i = 0; i < drive->audioTracks(); i++) {
        d->trackInfo.append(ParanoiaTrackInfoPtr(new ParanoiaTrackInfo(i)));
    }

    readCd();

    QAudioFormat format;
    format.setChannelCount(2);
    format.setSampleRate(44100);
    format.setSampleFormat(QAudioFormat::Int16);

    d->sink = new QAudioSink(format, this);
    d->sinkOutput = d->sink->start();
    d->sink->suspend();
    connect(d->player, &ParanoiaPlayer::frameAvailable, this, &ParanoiaCdController::feedSink);
    connect(d->player, &ParanoiaPlayer::epochChanged, this, [this] {
        d->sinkOutput = d->sink->start();
    });

    QTimer* sinkFeedTimer = new QTimer(this);
    sinkFeedTimer->setTimerType(Qt::PreciseTimer);
    sinkFeedTimer->setInterval(20);
    connect(sinkFeedTimer, &QTimer::timeout, this, &ParanoiaCdController::feedSink);
    sinkFeedTimer->start();

    StateManager::instance()->sources()->addSource(d->source);
}

ParanoiaCdController::~ParanoiaCdController() {
    StateManager::instance()->sources()->removeSource(d->source);
    delete d;
}

QString ParanoiaCdController::albumName() {
    return d->albumName;
}

QCoro::Task<> ParanoiaCdController::eject() {
    auto drive = d->disk->interface<BlockInterface>()->drive();
    co_await drive->eject();
}

MediaItem* ParanoiaCdController::mediaItem(int row) {
    return new ParanoiaMediaItem(d->player, row, d->sink, d->trackInfo.at(row));
}

void ParanoiaCdController::readCd() {
    if (!d->device.open(d->disk->interface<BlockInterface>()->blockName().toUtf8().constData(), DRIVER_DEVICE)) return;

    auto cdText = d->device.getCdtext();
    QMap<QString, QString> discFields;
    if (cdText) {
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
    }

    d->player = new ParanoiaPlayer(&d->device, this);

    emit dataChanged(index(0), index(rowCount() - 1));

    if (discFields.contains("TITLE")) {
        d->albumName = discFields.value("TITLE");
        emit albumNameChanged();
    }
}

void ParanoiaCdController::feedSink() {
    while (d->sink->bytesFree() >= 2342 && d->player->isFrameAvailable()) {
        d->sinkOutput->write(d->player->nextFrame(1));
    }
}

int ParanoiaCdController::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;

    return d->trackInfo.length();
}

QVariant ParanoiaCdController::data(const QModelIndex& index, int role) const {
    if (index.parent().isValid()) return {};

    auto track = d->trackInfo.at(index.row());
    switch (role) {
        case TitleRole:
            return track->title();
        case ArtistRole:
            return track->artist();
        case AlbumRole:
            return track->album();
        case TrackRole:
            return index.row() + 1;
    }

    return {};
}

QHash<int, QByteArray> ParanoiaCdController::roleNames() const {
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
