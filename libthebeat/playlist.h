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
#include "mediaitem.h"
#include <QObject>

struct PlaylistPrivate;
class LIBTHEBEAT_EXPORT Playlist : public QObject {
        Q_OBJECT
    public:
        enum State {
            Playing,
            Paused,
            Stopped
        };
        Q_ENUM(State)

    private:
        Q_PROPERTY(MediaItem* currentItem READ currentItem WRITE setCurrentItem NOTIFY currentItemChanged FINAL)
        Q_PROPERTY(State state READ state NOTIFY stateChanged FINAL)
        Q_PROPERTY(bool repeatOne READ repeatOne WRITE setRepeatOne NOTIFY repeatOneChanged FINAL)
        Q_PROPERTY(bool repeatAll READ repeatAll WRITE setRepeatAll NOTIFY repeatAllChanged FINAL)
        Q_PROPERTY(bool shuffle READ shuffle WRITE setShuffle NOTIFY shuffleChanged FINAL)
        Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged FINAL)
        Q_PROPERTY(double logAdjustedVolume READ logAdjustedVolume NOTIFY logAdjustedVolumeChanged FINAL)
        Q_PROPERTY(bool pauseAfterCurrentTrack READ pauseAfterCurrentTrack WRITE setPauseAfterCurrentTrack NOTIFY pauseAfterCurrentTrackChanged FINAL)

    public:
        explicit Playlist(QObject* parent = nullptr);

        Q_SCRIPTABLE void addItem(MediaItem* item);
        Q_SCRIPTABLE void removeItem(MediaItem* item);
        Q_SCRIPTABLE void insertItem(int index, MediaItem* item);
        MediaItem* takeItem(int index);
        Q_SCRIPTABLE void clear();

        State state();

        MediaItem* currentItem();
        void setCurrentItem(MediaItem* item);

        QList<MediaItem*> items();

        void setRepeatOne(bool repeatOne);
        bool repeatOne();

        void setRepeatAll(bool repeatAll);
        bool repeatAll();

        void setShuffle(bool shuffle);
        bool shuffle();

        void setVolume(double volume);
        double volume();
        double logAdjustedVolume();

        void setTrachChangeNotificationsEnabled(bool notificationsEnabled);

        void setPauseAfterCurrentTrack(bool pauseAfterCurrentTrack);
        bool pauseAfterCurrentTrack();

    public slots:
        void play();
        void playPause();
        void pause();
        void next();
        void previous();

    signals:
        void stateChanged(State newState, State oldState);
        void currentItemChanged(MediaItem* item);
        void itemsChanged();
        void metadataChanged();
        void repeatOneChanged(bool repeatOne);
        void repeatAllChanged(bool repeatAll);
        void shuffleChanged(bool shuffle);
        void volumeChanged(double volume);
        void logAdjustedVolumeChanged(double logAdjustedVolume);
        void pauseAfterCurrentTrackChanged(bool pauseAfterCurrentTrack);

    private:
        PlaylistPrivate* d;
        void updateMetadata();
};

#endif // PLAYLIST_H
