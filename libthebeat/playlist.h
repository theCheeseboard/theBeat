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
#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "libthebeat_global.h"
#include <QObject>
#include "mediaitem.h"

struct PlaylistPrivate;
class LIBTHEBEAT_EXPORT Playlist : public QObject {
        Q_OBJECT
    public:
        explicit Playlist(QObject* parent = nullptr);

        enum State {
            Playing,
            Paused,
            Stopped
        };

        void addItem(MediaItem* item);
        void removeItem(MediaItem* item);
        void clear();

        State state();

        MediaItem* currentItem();
        void setCurrentItem(MediaItem* item);

        QList<MediaItem*> items();

        void setRepeatOne(bool repeatOne);
        void setShuffle(bool shuffle);
        bool repeatOne();
        bool shuffle();

    public slots:
        void play();
        void pause();
        void next();
        void previous();

    signals:
        void stateChanged(State newState, State oldState);
        void currentItemChanged(MediaItem* item);
        void itemsChanged();
        void metadataChanged();
        void repeatOneChanged(bool repeatOne);
        void shuffleChanged(bool shuffle);

    private:
        PlaylistPrivate* d;
};

#endif // PLAYLIST_H
