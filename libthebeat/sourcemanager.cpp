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
#include "sourcemanager.h"

#include "pluginmediasource.h"

struct SourceManagerPrivate {
        QList<PluginMediaSource*> sources;
        int padTop;
};

SourceManager::SourceManager(QObject* parent) :
    QObject(parent) {
    d = new SourceManagerPrivate();
}

void SourceManager::addSource(PluginMediaSource* source) {
    if (d->sources.contains(source)) return;
    connect(source, &PluginMediaSource::destroyed, this, [this, source] {
        removeSource(source);
    });
    d->sources.append(source);
    emit sourceAdded(source);
}

void SourceManager::removeSource(PluginMediaSource* source) {
    if (!d->sources.contains(source)) return;
    d->sources.removeOne(source);
    emit sourceRemoved(source);
}

int SourceManager::padTop() {
    return d->padTop;
}

void SourceManager::setPadTop(int padTop) {
    d->padTop = padTop;
}
