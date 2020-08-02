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
#ifndef LIBRARYMANAGER_H
#define LIBRARYMANAGER_H

#include <QObject>
#include "librarymodel.h"

class QSqlQuery;
struct LibraryManagerPrivate;
class LibraryManager : public QObject {
        Q_OBJECT
    public:
        explicit LibraryManager(QObject* parent = nullptr);

        struct LibraryTrack {
            QString path;
            QString title;
            QString artist;
            QString album;
            qint64 duration;
            int trackNumber;
        };

        static LibraryManager* instance();

        void enumerateDirectory(QString path, bool ignoreBlacklist = true);
        void addTrack(QString path);
        void removeTrack(QString path);
        void blacklistTrack(QString path);
        void relocateTrack(QString oldPath, QString newPath);

        LibraryModel* allTracks();
        LibraryModel* searchTracks(QString query);
        int countTracks();

        QStringList artists();
        QStringList albums();

        LibraryModel* tracksByArtist(QString artist);
        LibraryModel* tracksByAlbum(QString album);

        void createPlaylist(QString playlistName);
        QList<QPair<int, QString>> playlists();

        void addTrackToPlaylist(int playlist, QString path);
        LibraryModel* tracksByPlaylist(int playlist);

        bool isProcessing();

        void erase();

    signals:
        void libraryChanged();
        bool isProcessingChanged();
        void playlistsChanged();
        void playlistChanged(int id);

    private:
        LibraryManagerPrivate* d;
};

#endif // LIBRARYMANAGER_H
