#include "paranoiacdcontroller.h"

#include "paranoiacdpluginmediasource.h"
#include "paranoiamediaitem.h"
#include "paranoiaplayer.h"
#include "paranoiatrackinfo.h"
// #include <mediaitem/paranoiacdplayback.h>
#include <musicbrainzclient.h>
#include <pluginmediasource.h>
#include <sourcemanager.h>
#include <QTimer>
#include <QtConcurrent>
#include <QCoroFuture>
#include <statemanager.h>

#include <QAudioSink>
#include <QQmlEngine>

#include <cdio++/cdio.hpp>

struct ParanoiaCdControllerPrivate {
    QString deviceDescriptor;
    PluginMediaSource* source;
    ParanoiaPlayer* player = nullptr;
    QAudioSink* sink = nullptr;
    QIODevice* sinkOutput = nullptr;
    MusicBrainzClient* musicBrainzClient = nullptr;

    QString albumName;
    QList<ParanoiaTrackInfoPtr> trackInfo;
    CdioDevice device;
};

ParanoiaCdController::ParanoiaCdController(QString deviceDescriptor, QWidget* parent) :
    QAbstractListModel(parent) {
    d = new ParanoiaCdControllerPrivate();

    d->deviceDescriptor = deviceDescriptor;
    d->albumName = tr("CD");

    d->source = new ParanoiaCdPluginMediaSource(this);
    d->source->setIcon(QIcon::fromTheme("media-optical-audio"));

    QAudioFormat format;
    format.setChannelCount(2);
    format.setSampleRate(44100);
    format.setSampleFormat(QAudioFormat::Int16);

    d->sink = new QAudioSink(format, this);
    d->sinkOutput = d->sink->start();
    d->sink->suspend();

    this->openCd();

    QTimer* sinkFeedTimer = new QTimer(this);
    sinkFeedTimer->setTimerType(Qt::PreciseTimer);
    sinkFeedTimer->setInterval(20);
    connect(sinkFeedTimer, &QTimer::timeout, this, &ParanoiaCdController::feedSink);
    sinkFeedTimer->start();
}

ParanoiaCdController::~ParanoiaCdController() {
    StateManager::instance()->sources()->removeSource(d->source);
    delete d;
}

QString ParanoiaCdController::albumName() {
    return d->albumName;
}

MusicBrainzClient* ParanoiaCdController::musicBrainzClient() {
    return d->musicBrainzClient;
}

QCoro::Task<> ParanoiaCdController::eject() {
    // auto drive = d->disk->interface<BlockInterface>()->drive();
    // co_await drive->eject();
    d->device.ejectMedia();
    co_return;
}

MediaItem* ParanoiaCdController::mediaItem(int row) {
    auto item = new ParanoiaMediaItem(d->player, row, d->sink, d->trackInfo.at(row));
    StateManager::instance()->qmlEngine()->setObjectOwnership(item, QQmlEngine::CppOwnership);
    return item;
}

QCoro::Task<> ParanoiaCdController::openCd() {
    auto deviceOpen = co_await QtConcurrent::run([](CdioDevice * device, QString deviceDescriptor) {
        return device->open(deviceDescriptor.toUtf8().constData(), DRIVER_DEVICE);
    }, &d->device, d->deviceDescriptor);

    if (deviceOpen) {
        readCd();
    }
}

void ParanoiaCdController::readCd() {
    auto firstTrack = d->device.getFirstTrackNum();
    auto lastTrack = d->device.getLastTrackNum();
    // Probably not an audio CD so we don't really care
    if (firstTrack == lastTrack) return;

    // auto drive = d->disk->interface<BlockInterface>()->drive();
    for (auto i = firstTrack; i <= lastTrack; i++) {
        d->trackInfo.append(ParanoiaTrackInfoPtr(new ParanoiaTrackInfo(i - 1)));
    }

    this->setCdTextMetadata();
    d->player = new ParanoiaPlayer(&d->device, this);
    connect(d->player, &ParanoiaPlayer::frameAvailable, this, &ParanoiaCdController::feedSink);
    connect(d->player, &ParanoiaPlayer::epochChanged, this, [this] {
        if (d->sink->state() != QAudio::SuspendedState) {
            d->sinkOutput = d->sink->start();
        }
    });
    this->setupMusicBrainzClient();
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

    emit dataChanged(index(0), index(rowCount() - 1));

    StateManager::instance()->sources()->addSource(d->source);
}

void ParanoiaCdController::feedSink() {
    while (d->sink->bytesFree() >= 2342 && d->player && d->player->isFrameAvailable() && d->sink->state() != QAudio::SuspendedState) {
        d->sinkOutput->write(d->player->nextFrame(1));
    }
}

void ParanoiaCdController::setCdTextMetadata() {
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

    if (discFields.contains("TITLE")) {
        d->albumName = discFields.value("TITLE");
        emit albumNameChanged();
    }
}

void ParanoiaCdController::setupMusicBrainzClient() {
    auto firstTrack = d->device.getFirstTrackNum();
    auto lastTrack = d->device.getLastTrackNum();
    auto leadOut = d->device.getLastTrack()->getLastLsn() + 151;
    int frameOffsets[99];
    for (int i = 0; i < 99; i++) {
        int frameOffset = 0;
        if (i + 1 >= firstTrack && i + 1 <= lastTrack) {
            auto track = d->device.getTrackFromNum(i + 1);
            frameOffset = track->getLba();
        }
        frameOffsets[i] = frameOffset;
    }
    d->musicBrainzClient = new MusicBrainzClient(firstTrack, lastTrack, leadOut, frameOffsets, this);
    emit musicBrainzClientChanged();
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
