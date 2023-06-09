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

#include "librarymodel.h"
#include <QObject>

class QSqlQuery;
struct LibraryManagerPrivate;
class LibraryManager : public QObject {
        Q_OBJECT
    public:
        explicit LibraryManager(QObject* parent = nullptr);
        ~LibraryManager();

        enum SmartPlaylist : int {
            Frequents = 0,
            Random,
            LastSmartPlaylist
        };

        struct LibraryTrack {
                QString path;
                QString title;
                QString artist;
                QString album;
                qint64 duration;
                int trackNumber;
        };

        static LibraryManager* instance();

        void enumerateDirectory(QString path, bool ignoreBlacklist = true, bool isUserAction = false);
        void addTrack(QString path, bool updateOnly = false);
        void removeTrack(QString path);
        void blacklistTrack(QString path);
        void relocateTrack(QString oldPath, QString newPath);

        void bumpTrackPlayCount(QString path);
        int trackPlayCount(QString path);

        LibraryModel* allTracks();
        LibraryModel* searchTracks(QString query);
        int countTracks();

        QStringList artists();
        QStringList albums();

        LibraryModel* tracksByArtist(QString artist);
        LibraryModel* tracksByAlbum(QString album);

        int createPlaylist(QString playlistName);
        QList<QPair<int, QString>> playlists();
        void removePlaylist(int playlist);
        void renamePlaylist(int playlist, QString name);

        void addTrackToPlaylist(int playlist, QString path);
        void removeTrackFromPlaylist(int playlist, int sort);
        LibraryModel* tracksByPlaylist(int playlist);
        void normalisePlaylistSort(int playlist);

        LibraryModel* smartPlaylist(SmartPlaylist smartPlaylist);
        QString smartPlaylistName(SmartPlaylist smartPlaylist);

        bool isProcessing();

        void erase();

    signals:
        void libraryChanged();
        bool isProcessingChanged();
        void playlistsChanged();
        void playlistChanged(int id);

    private:
        LibraryManagerPrivate* d;

        int trackId(QString path);
};

struct TemporaryDatabase {
        QString dbName;
        QSqlDatabase db;

        TemporaryDatabase();
        ~TemporaryDatabase();
};

#endif // LIBRARYMANAGER_H
