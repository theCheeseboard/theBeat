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
#include "burnmanager.h"

#include "burnbackend.h"
#include <tlogger.h>

struct BurnManagerPrivate {
    QList<BurnBackend*> backends;
};

BurnManager::BurnManager(QObject* parent) : QObject(parent) {
    d = new BurnManagerPrivate();
}

void BurnManager::registerBackend(BurnBackend* backend) {
    connect(backend, &BurnBackend::destroyed, this, [ = ] {
        deregisterBackend(backend);
    });
    d->backends.append(backend);
    emit backendRegistered(backend);
}

void BurnManager::deregisterBackend(BurnBackend* backend) {
    backend->disconnect(this);
    d->backends.removeOne(backend);
    emit backendDeregistered(backend);
}

QList<BurnBackend*> BurnManager::availableBackends() {
    return d->backends;
}
