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
#include "burndevice.h"

#include "burnpopover.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusObjectPath>
#include <burnmanager.h>
#include <statemanager.h>
#include <tpopover.h>
#include <tpromise.h>

#include <DriveObjects/blockinterface.h>
#include <DriveObjects/diskobject.h>
#include <DriveObjects/driveinterface.h>

struct BurnDevicePrivate {
        bool registered = false;

        QString displayName;
        DiskObject* diskObject;
};

BurnDevice::BurnDevice(DiskObject* diskObject, QObject* parent) :
    BurnBackend(parent) {
    d = new BurnDevicePrivate();
    d->diskObject = diskObject;

    // Get CD drive and attach to CD information
    connect(d->diskObject->interface<BlockInterface>()->drive(), &DriveInterface::changed, this, &BurnDevice::checkCd);

    checkCd();
}

BurnDevice::~BurnDevice() {
    if (d->registered) StateManager::instance()->burn()->deregisterBackend(this);
    delete d;
}

void BurnDevice::checkCd() {
    DriveInterface* drive = d->diskObject->interface<BlockInterface>()->drive();
    d->displayName = QStringLiteral("%1 %2").arg(drive->vendor()).arg(drive->model());

    bool available = drive->mediaAvailable() && drive->isOpticalDrive() && QList<DriveInterface::MediaFormat>({DriveInterface::CdR, DriveInterface::CdRw}).contains(drive->media());
    if (available && !d->registered) {
        // Register
        StateManager::instance()->burn()->registerBackend(this);
        d->registered = true;
    } else if (!available && d->registered) {
        // Deregister
        StateManager::instance()->burn()->deregisterBackend(this);
        d->registered = false;
    }
}

void BurnDevice::burn(QStringList files, QString albumName, QWidget* parentWindow) {
    BurnPopover* jp = new BurnPopover(files, d->diskObject, albumName);
    tPopover* popover = new tPopover(jp);
    popover->setPopoverWidth(SC_DPI(-200));
    popover->setPopoverSide(tPopover::Bottom);
    connect(jp, &BurnPopover::done, popover, &tPopover::dismiss);
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    connect(popover, &tPopover::dismissed, jp, &BurnPopover::deleteLater);
    popover->show(parentWindow->window());
}

QString BurnDevice::displayName() {
    return d->displayName;
}
