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
#ifndef MEDIAITEM_H
#define MEDIAITEM_H

#include "libthebeat_global.h"
#include <QObject>
#include <QSharedPointer>

class LIBTHEBEAT_EXPORT MediaItem : public QObject {
        Q_OBJECT
    public:
        explicit MediaItem();

        virtual void play() = 0;
        virtual void pause() = 0;
        virtual void seek(quint64 ms) = 0;

        virtual QString title() = 0;
        virtual QStringList authors() = 0;
        virtual QString album() = 0;
        virtual QImage albumArt() = 0;

    signals:
        void done();
        void metadataChanged();

    private:

};

#endif // MEDIAITEM_H
