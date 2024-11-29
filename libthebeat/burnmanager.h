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
#ifndef BURNMANAGER_H
#define BURNMANAGER_H

#include "libthebeat_global.h"
#include <QObject>

class BurnBackend;
struct BurnManagerPrivate;
class LIBTHEBEAT_EXPORT BurnManager : public QObject {
        Q_OBJECT
        Q_PROPERTY(QList<BurnBackend*> availableBackends READ availableBackends NOTIFY availableBackendsChanged FINAL)
    public:
        explicit BurnManager(QObject* parent = nullptr);

        void registerBackend(BurnBackend* backend);
        void deregisterBackend(BurnBackend* backend);

        QList<BurnBackend*> availableBackends();

    signals:
        void backendRegistered(BurnBackend* backend);
        void backendDeregistered(BurnBackend* backend);
        void availableBackendsChanged();

    private:
        BurnManagerPrivate* d;
};

#endif // BURNMANAGER_H
