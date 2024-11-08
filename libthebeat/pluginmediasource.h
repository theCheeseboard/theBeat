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
#ifndef PLUGINMEDIASOURCE_H
#define PLUGINMEDIASOURCE_H

#include "libthebeat_global.h"
#include <QIcon>
#include <QUrl>
#include <QWidget>
#include <abstractlibrarybrowser.h>

struct PluginMediaSourcePrivate;
class LIBTHEBEAT_EXPORT PluginMediaSource : public QObject {
        Q_OBJECT
        Q_PROPERTY(QUrl qmlFile READ qmlFile FINAL CONSTANT)
        Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
        Q_PROPERTY(QIcon icon READ icon WRITE setIcon NOTIFY iconChanged FINAL)

    public:
        explicit PluginMediaSource(AbstractLibraryBrowser* widget, QUrl qmlFile = QUrl(), QObject* parent = nullptr);
        ~PluginMediaSource();

        void setName(QString name);
        QString name() const;

        void setIcon(QIcon icon);
        QIcon icon() const;

        AbstractLibraryBrowser* widget() const;
        QUrl qmlFile() const;

    signals:
        void nameChanged(QString name);
        void iconChanged(QIcon icon);

    private:
        PluginMediaSourcePrivate* d;
};

#endif // PLUGINMEDIASOURCE_H
