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
#include "pluginmediasource.h"

struct PluginMediaSourcePrivate {
    AbstractLibraryBrowser* widget;
    QString name;
    QIcon icon;
};

PluginMediaSource::PluginMediaSource(AbstractLibraryBrowser* widget, QObject* parent) : QObject(parent) {
    d = new PluginMediaSourcePrivate();
    d->widget = widget;
}

PluginMediaSource::~PluginMediaSource() {
    delete d;
}

void PluginMediaSource::setName(QString name) {
    d->name = name;
    emit nameChanged(name);
}

QString PluginMediaSource::name() const {
    return d->name;
}

void PluginMediaSource::setIcon(QIcon icon) {
    d->icon = icon;
    emit iconChanged(icon);
}

QIcon PluginMediaSource::icon() const {
    return d->icon;
}

AbstractLibraryBrowser* PluginMediaSource::widget() const {
    return d->widget;
}
