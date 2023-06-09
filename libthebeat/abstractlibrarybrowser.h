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
#ifndef ABSTRACTLIBRARYBROWSER_H
#define ABSTRACTLIBRARYBROWSER_H

#include "libthebeat_global.h"
#include <QWidget>

class LIBTHEBEAT_EXPORT AbstractLibraryBrowser : public QWidget
{
        Q_OBJECT
    public:
        AbstractLibraryBrowser(QWidget* parent = nullptr) : QWidget(parent) {}
        virtual ~AbstractLibraryBrowser() {}

        struct TrackInformation {
                QString title;
                QString artist;
                QString album;
                int trackNumber;
                quint64 duration;
        };

        struct ListInformation {
                QString name;
                QImage graphic;
                QList<TrackInformation> tracks;
        };

        virtual ListInformation currentListInformation() = 0;
};

#endif // ABSTRACTLIBRARYBROWSER_H
