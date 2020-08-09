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

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusObjectPath>
#include <statemanager.h>
#include <burnmanager.h>
#include <tpromise.h>
#include <tpopover.h>
#include "burnpopover.h"

struct BurnDevicePrivate {
    QDBusObjectPath cdDrivePath;
    bool registered = false;

    QString displayName;
    QString blockDevice;
};

BurnDevice::BurnDevice(QString blockDevice, QObject* parent) : BurnBackend(parent) {
    d = new BurnDevicePrivate();
    d->blockDevice = blockDevice;

    //Get CD drive and attach to CD information
    QDBusInterface blockInterface("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2/block_devices/" + blockDevice, "org.freedesktop.UDisks2.Block", QDBusConnection::systemBus());
    d->cdDrivePath = blockInterface.property("Drive").value<QDBusObjectPath>();
    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", d->cdDrivePath.path(), "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(checkCd()));

    checkCd();
}

BurnDevice::~BurnDevice() {
    delete d;
}

void BurnDevice::checkCd() {
    struct CdInformation {
        bool available = false;
        QString displayName;
    };

    tPromise<CdInformation>::runOnNewThread([ = ](tPromiseFunctions<CdInformation>::SuccessFunction res, tPromiseFunctions<CdInformation>::FailureFunction rej) {
        if (d->cdDrivePath.path() == "") {
            res(CdInformation());
            return;
        }

        QDBusInterface cdDriveInterface("org.freedesktop.UDisks2", d->cdDrivePath.path(), "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus());
        if (cdDriveInterface.property("MediaAvailable").toBool() && cdDriveInterface.property("Optical").toBool() && cdDriveInterface.property("OpticalBlank").toBool()) {
            CdInformation info;
            info.available = true;
            info.displayName = QStringLiteral("%1 %2").arg(cdDriveInterface.property("Vendor").toString()).arg(cdDriveInterface.property("Model").toString());
            res(info);
        } else {
            res(CdInformation());
        }
    })->then([ = ](CdInformation info) {
        d->displayName = info.displayName;
        if (info.available && !d->registered) {
            //Register
            StateManager::instance()->burn()->registerBackend(this);
            d->registered = true;
        } else if (!info.available && d->registered) {
            //Deregister
            StateManager::instance()->burn()->deregisterBackend(this);
            d->registered = false;
        }
    });
}

void BurnDevice::burn(QStringList files, QString albumName, QWidget* parentWindow) {
    BurnPopover* jp = new BurnPopover(files, d->blockDevice, albumName);
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
