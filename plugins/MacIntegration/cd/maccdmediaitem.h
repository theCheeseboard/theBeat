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
#ifndef MACCDMEDIAITEM_H
#define MACCDMEDIAITEM_H

#include <mediaitem.h>
#include "trackinfo.h"

struct MacCdMediaItemPrivate;
class MacCdMediaItem : public MediaItem {
        Q_OBJECT
    public:
        explicit MacCdMediaItem(QString volume, TrackInfoPtr info);
        ~MacCdMediaItem();

        static void volumeGone(QString volume);

    signals:

    private:
        MacCdMediaItemPrivate* d;

        // MediaItem interface
    public:
        void play();
        void pause();
        void stop();
        void seek(quint64 ms);
        quint64 elapsed();
        quint64 duration();
        QString title();
        QStringList authors();
        QString album();
        QImage albumArt();
        QVariant metadata(QString key);
};

#endif // PHONONCDMEDIAITEM_H
