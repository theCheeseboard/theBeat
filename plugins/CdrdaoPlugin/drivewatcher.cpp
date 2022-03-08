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
#include "drivewatcher.h"

#include "burndevice.h"
#include <DriveObjects/diskobject.h>
#include <driveobjectmanager.h>
#include <tapplication.h>

struct DriveWatcherPrivate {
        QMap<DiskObject*, BurnDevice*> devices;
};

DriveWatcher::DriveWatcher(QObject* parent) :
    QObject{parent} {
    d = new DriveWatcherPrivate();

    connect(DriveObjectManager::instance(), &DriveObjectManager::diskAdded, this, [=](DiskObject* disk) {
        update();
    });
    connect(DriveObjectManager::instance(), &DriveObjectManager::diskRemoved, this, [=](DiskObject* disk) {
        update();
    });
    update();
}

DriveWatcher::~DriveWatcher() {
    delete d;
}

void DriveWatcher::update() {
    if (tApplication::currentPlatform() == tApplication::Flatpak) return; // Burning is not supported in flatpak

    QList<DiskObject*> opticalDisks = DriveObjectManager::opticalDisks();
    for (DiskObject* diskObject : opticalDisks) {
        if (d->devices.contains(diskObject)) continue;

        BurnDevice* burnDevice = new BurnDevice(diskObject, this);
        d->devices.insert(diskObject, burnDevice);
    }

    QList<DiskObject*> toRemove;
    for (DiskObject* diskObject : d->devices.keys()) {
        if (!opticalDisks.contains(diskObject)) toRemove.append(diskObject);
    }

    for (DiskObject* diskObject : toRemove) {
        BurnDevice* burnDevice = d->devices.take(diskObject);
        burnDevice->deleteLater();
    }
}
