#include "paranoiaplayer.h"

#include <QAudioSink>
#include <QMutex>
#include <QQueue>
#include <QThread>
#include <QTimer>
#include <tlogger.h>

#include <cdio++/cdio.hpp>
#include <cdio/paranoia/paranoia.h>

struct ParanoiaPlayerPrivate {
        QThread workerThread;
        ParanoiaPlayerWorker* worker;

        QTimer* seekDebounce;
        int targetTrack;
        quint64 targetMs;
        quint64 position;
};

ParanoiaPlayer::ParanoiaPlayer(CdioDevice* cdio, QObject* parent) :
    QObject{parent}, d{new ParanoiaPlayerPrivate} {
    d->worker = new ParanoiaPlayerWorker(cdio);
    d->worker->moveToThread(&d->workerThread);
    connect(&d->workerThread, &QThread::started, d->worker, &ParanoiaPlayerWorker::prepare);
    connect(&d->workerThread, &QThread::finished, d->worker, &ParanoiaPlayerWorker::deleteLater);
    d->workerThread.start();

    connect(d->worker, &ParanoiaPlayerWorker::frameAvailable, this, &ParanoiaPlayer::frameAvailable);
    connect(d->worker, &ParanoiaPlayerWorker::epochChanged, this, &ParanoiaPlayer::epochChanged);
    connect(d->worker, &ParanoiaPlayerWorker::frameRead, this, &ParanoiaPlayer::handleFrameRead);

    d->seekDebounce = new QTimer(this);
    d->seekDebounce->setInterval(50);
    d->seekDebounce->setSingleShot(true);
    connect(d->seekDebounce, &QTimer::timeout, this, [this] {
        d->worker->metaObject()->invokeMethod(d->worker, "seek", Qt::QueuedConnection, Q_ARG(int, d->targetTrack), Q_ARG(quint64, d->targetMs));
    });
}

ParanoiaPlayer::~ParanoiaPlayer() {
    delete d;
}

QByteArray ParanoiaPlayer::nextFrame(int frameCount) {
    return d->worker->nextFrame(frameCount);
}

bool ParanoiaPlayer::isFrameAvailable() {
    return d->worker->isFrameAvailable();
}

ParanoiaPlayer::Track ParanoiaPlayer::track(int track) {
    return d->worker->track(track);
}

void ParanoiaPlayer::seek(int track, quint64 ms) {
    // Debounce the seek
    d->targetTrack = track;
    d->targetMs = ms;
    if (d->seekDebounce->isActive()) d->seekDebounce->stop();
    d->seekDebounce->start();
}

void ParanoiaPlayer::handleFrameRead(lba_t position) {
    // Freeze the position if we're debouncing a seek
    if (!d->seekDebounce->isActive()) {
        d->position = position;
    }
    emit frameRead(d->position);
}

struct ParanoiaPlayerWorkerPrivate {
        CdioDevice* cdio;
        cdrom_paranoia_t* paranoia;
        QQueue<QByteArray> frames;
        mutable QMutex framesMutex;
        bool reading = false;
        quint32 epoch = 0;

        QList<ParanoiaPlayer::Track> tracks;
        lsn_t position = 0;
};

ParanoiaPlayerWorker::ParanoiaPlayerWorker(CdioDevice* cdio) :
    QObject(nullptr), d{new ParanoiaPlayerWorkerPrivate} {
    d->cdio = cdio;
}

ParanoiaPlayerWorker::~ParanoiaPlayerWorker() {
    cdio_paranoia_free(d->paranoia);
    delete d;
}

QByteArray ParanoiaPlayerWorker::nextFrame(int frameCount) {
    QMutexLocker locker(&d->framesMutex);
    if (d->frames.isEmpty()) return {};
    QByteArray frames;
    for (auto i = 0; i < frameCount; i++) {
        frames.append(d->frames.dequeue());
        d->position++;
        if (d->frames.isEmpty()) break;
    }
    this->metaObject()->invokeMethod(this, "tryReadNextFrame", Qt::QueuedConnection);
    emit frameRead(d->position);
    tDebug("ParanoiaPlayerWorker") << "Frames left: " << d->frames.length();
    return frames;
}

bool ParanoiaPlayerWorker::isFrameAvailable() {
    QMutexLocker locker(&d->framesMutex);
    return !d->frames.isEmpty();
}

ParanoiaPlayer::Track ParanoiaPlayerWorker::track(int track) {
    QMutexLocker locker(&d->framesMutex);
    return d->tracks.at(track);
}

lsn_t ParanoiaPlayerWorker::position() {
    QMutexLocker locker(&d->framesMutex);
    return d->position;
}

void ParanoiaPlayerWorker::prepare() {
    auto firstTrack = d->cdio->getFirstTrackNum();
    auto lastTrack = d->cdio->getLastTrackNum();

    for (auto i = firstTrack; i <= lastTrack; i++) {
        auto track = d->cdio->getTrackFromNum(i);
        d->tracks.append({.firstLsn = track->getLsn(),
            .lastLsn = track->getLastLsn()});
    }

    auto drive = cdio_cddap_identify_cdio(d->cdio->getCdIo(), CDDA_MESSAGE_PRINTIT, nullptr);
    cdio_cddap_open(drive);
    auto firstSector = cdda_disc_firstsector(drive);
    cdda_verbose_set(drive, CDDA_MESSAGE_PRINTIT, CDDA_MESSAGE_PRINTIT);

    d->paranoia = cdio_paranoia_init(drive);
    cdio_paranoia_modeset(d->paranoia, PARANOIA_MODE_FULL);
    cdio_paranoia_seek(d->paranoia, firstSector, SEEK_SET);
    d->position = firstSector;

    // Kick off reading the CD
    this->tryReadNextFrame();
}

void ParanoiaPlayerWorker::seek(int track, quint64 ms) {
    auto extraSectors = ms / 13.333;
    auto lsn = d->tracks.at(track).firstLsn + extraSectors;
    cdio_paranoia_seek(d->paranoia, lsn, SEEK_SET);
    d->position = lsn;
    this->bumpEpoch();
}

void ParanoiaPlayerWorker::tryReadNextFrame() {
    if (d->reading) return;
    d->reading = true;
    auto thisEpoch = d->epoch;
    while (d->frames.length() < 750) {
        auto buf = cdio_paranoia_read(d->paranoia, nullptr);

        {
            QMutexLocker locker(&d->framesMutex);
            if (thisEpoch != d->epoch) return;
            if (!buf) {
                // Couldn't read anything - bail out now
                d->reading = false;
                return;
            }
            d->frames.enqueue(QByteArray(reinterpret_cast<const char*>(buf), CDIO_CD_FRAMESIZE_RAW));
        }

        emit frameAvailable();
    }
    d->reading = false;
}

void ParanoiaPlayerWorker::bumpEpoch() {
    d->epoch++;
    d->reading = false;

    {
        QMutexLocker locker(&d->framesMutex);
        d->frames.clear();
    }

    emit epochChanged();
    this->tryReadNextFrame();
}
