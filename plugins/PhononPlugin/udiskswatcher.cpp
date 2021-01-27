/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2021 Victor Tran
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
#include "udiskswatcher.h"

#include <QMap>
#include <QSet>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusArgument>
#include <QDBusInterface>
#include "cdchecker.h"

struct UdisksWatcherPrivate {
    QMap<QDBusObjectPath, CdChecker*> checkers;
    QSet<QDBusObjectPath> opticalDrives;

    const QStringList opticalFormats = {
        "optical",
        "optical_cd",
        "optical_cd_r",
        "optical_cd_rw"
    };
};

UdisksWatcher::UdisksWatcher(QObject* parent) : QObject(parent) {
    d = new UdisksWatcherPrivate();

    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", "InterfacesAdded", this, SLOT(updateInterfaces()));
    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", "InterfacesRemoved", this, SLOT(updateInterfaces()));
    updateInterfaces();
}

UdisksWatcher::~UdisksWatcher() {
    delete d;
}

void UdisksWatcher::updateInterfaces() {
    QDBusMessage managedObjects = QDBusMessage::createMethodCall("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");
    QMap<QDBusObjectPath, QMap<QString, QVariantMap>> reply;
    QDBusArgument arg = QDBusConnection::systemBus().call(managedObjects).arguments().first().value<QDBusArgument>();

    arg >> reply;

    //First, detect all the drives
    for (const QDBusObjectPath& path : reply.keys()) {
        auto interfaces = reply.value(path);

        //Determine which type of object to create
        if (path.path().startsWith("/org/freedesktop/UDisks2/drives")) {
            QDBusInterface interface("org.freedesktop.UDisks2", path.path(), "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus());
            QStringList compatibility = interface.property("MediaCompatibility").toStringList();
            bool isDriveOptical = false;
            for (const QString& compatible : qAsConst(compatibility)) {
                if (d->opticalFormats.contains(compatible)) isDriveOptical = true;
            }
            if (d->opticalDrives.contains(path) && !isDriveOptical) {
                d->opticalDrives.remove(path);
            } else if (!d->opticalDrives.contains(path) && isDriveOptical) {
                d->opticalDrives.insert(path);
            }
        }
    }

    //Now detect all the block devices
    for (const QDBusObjectPath& path : reply.keys()) {
        auto interfaces = reply.value(path);

        //Determine which type of object to create
        if (path.path().startsWith("/org/freedesktop/UDisks2/block_devices")) {
            QDBusInterface interface("org.freedesktop.UDisks2", path.path(), "org.freedesktop.UDisks2.Block", QDBusConnection::systemBus());
            QDBusObjectPath drive = interface.property("Drive").value<QDBusObjectPath>();
            bool isOptical = d->opticalDrives.contains(drive);
            if (d->checkers.contains(path) && !isOptical) {
                d->checkers.take(path)->deleteLater();
            } else if (!d->checkers.contains(path) && isOptical) {
                d->checkers.insert(path, new CdChecker(path));
            }
        }
    }

    QList<QDBusObjectPath> goneBlocks;
    for (const QDBusObjectPath& path : d->checkers.keys()) {
        if (!reply.contains(path)) goneBlocks.append(path);
    }
    QList<QDBusObjectPath> goneDrives;
    for (const QDBusObjectPath& path : qAsConst(d->opticalDrives)) {
        if (!reply.contains(path)) goneDrives.append(path);
    }

    for (const QDBusObjectPath& path : goneBlocks) {
        d->checkers.take(path)->deleteLater();
    }
    for (const QDBusObjectPath& path : goneDrives) {
        d->opticalDrives.remove(path);
    }

}
