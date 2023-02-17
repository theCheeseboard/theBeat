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
#ifndef SOURCEMANAGER_H
#define SOURCEMANAGER_H

#include "libthebeat_global.h"
#include <QObject>

class PluginMediaSource;
struct SourceManagerPrivate;
class LIBTHEBEAT_EXPORT SourceManager : public QObject {
        Q_OBJECT
    public:
        explicit SourceManager(QObject* parent = nullptr);

        void addSource(PluginMediaSource* source);
        void removeSource(PluginMediaSource* source);

        QList<PluginMediaSource*> sources();

        int padTop();
        void setPadTop(int padTop);

    signals:
        void sourceAdded(PluginMediaSource* source);
        void sourceRemoved(PluginMediaSource* source);
        void padTopChanged(int padTop);

    private:
        SourceManagerPrivate* d;
};

#endif // SOURCEMANAGER_H
