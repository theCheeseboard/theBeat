/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2022 Victor Tran
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
#include "fullburnjob.h"

#include <DriveObjects/blockinterface.h>
#include <DriveObjects/diskobject.h>
#include <DriveObjects/driveinterface.h>
#include <DriveObjects/filesysteminterface.h>

#include <QCoroProcess>
#include <frisbeeexception.h>

struct FullBurnJobPrivate {
        DiskObject* diskObject;
        AbstractBurnJob* mainBurnJob;
        bool shouldUnlock = false;

        bool isErasing = false;
        bool erasingFailed = false;
};

FullBurnJob::FullBurnJob(DiskObject* diskObject, AbstractBurnJob* mainBurnJob, QObject* parent) :
    AbstractBurnJob{parent} {
    d = new FullBurnJobPrivate();
    d->diskObject = diskObject;
    d->mainBurnJob = mainBurnJob;

    connect(d->mainBurnJob, &AbstractBurnJob::canCancelChanged, this, &FullBurnJob::canCancelChanged);
    connect(d->mainBurnJob, &AbstractBurnJob::descriptionChanged, this, &FullBurnJob::descriptionChanged);
    connect(d->mainBurnJob, &AbstractBurnJob::progressChanged, this, &FullBurnJob::progressChanged);
    connect(d->mainBurnJob, &AbstractBurnJob::totalProgressChanged, this, &FullBurnJob::totalProgressChanged);
    connect(d->mainBurnJob, &AbstractBurnJob::stateChanged, this, [this](AbstractBurnJob::State state) {
        emit stateChanged(state);

        if (!d->shouldUnlock) return;
        if (state == AbstractBurnJob::Finished || state == AbstractBurnJob::Failed) {
            d->diskObject->releaseLock();
            d->shouldUnlock = false;
        }
    });
}

FullBurnJob::~FullBurnJob() {
    d->mainBurnJob->deleteLater();
    delete d;
}

QCoro::Task<> FullBurnJob::unmountAndEraseDisc() {
    d->isErasing = true;
    emit stateChanged(state());
    emit canCancelChanged(canCancel());
    emit descriptionChanged(description());

    FilesystemInterface* fs = d->diskObject->interface<FilesystemInterface>();
    if (!fs) {
        // This disc does not have a filesystem so doesn't need to be unmounted
        co_await eraseDisc();
        co_return;
    }

    QByteArrayList mountPoints = fs->mountPoints();
    if (mountPoints.count() == 0) {
        // This disc is not mounted
        co_await eraseDisc();
        co_return;
    }

    // Unmount the disc
    try {
        co_await fs->unmount();
        eraseDisc();
    } catch (FrisbeeException& ex) {
        d->isErasing = false;
        d->erasingFailed = true;

        // Bail out
        emit stateChanged(Failed);
        emit descriptionChanged(description());
    }
}

QCoro::Task<> FullBurnJob::eraseDisc() {
    // Erase the disc
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);

    QStringList cdrecordArgs = {"blank", "--device", d->diskObject->interface<BlockInterface>()->blockName(), "--blank-mode", "minimal"};
    proc.start("cdrdao", cdrecordArgs);

    co_await proc;

    if (proc.exitCode() == 0) {
        // Start the main burn job after reloading the disc
        co_await d->diskObject->interface<BlockInterface>()->triggerReload();
        d->mainBurnJob->start();
        d->isErasing = false;
        emit stateChanged(state());
        emit canCancelChanged(canCancel());
        emit descriptionChanged(description());
    } else {
        d->isErasing = false;
        d->erasingFailed = true;

        // Bail out
        emit stateChanged(Failed);
        emit descriptionChanged(description());
    }
}

quint64 FullBurnJob::progress() {
    if (d->isErasing) return 0;
    if (d->erasingFailed) return 1;
    return d->mainBurnJob->progress();
}

quint64 FullBurnJob::totalProgress() {
    if (d->isErasing) return 0;
    if (d->erasingFailed) return 1;
    return d->mainBurnJob->totalProgress();
}

tJob::State FullBurnJob::state() {
    if (d->erasingFailed) return tJob::Failed;
    if (d->isErasing) return tJob::Processing;
    return d->mainBurnJob->state();
}

QCoro::Task<> FullBurnJob::start() {
    co_await d->diskObject->lock();
    d->shouldUnlock = true;

    if (!d->diskObject->interface<BlockInterface>()->drive()->opticalBlank()) {
        this->unmountAndEraseDisc();
    } else {
        d->mainBurnJob->start();
    }
}

QString FullBurnJob::description() {
    if (d->erasingFailed) return tr("Failed to erase the disc before burning");
    if (d->isErasing) return tr("Erasing disc");
    return d->mainBurnJob->description();
}

bool FullBurnJob::canCancel() {
    if (d->isErasing) return false;
    return d->mainBurnJob->canCancel();
}

void FullBurnJob::cancel() {
    d->mainBurnJob->cancel();
}
