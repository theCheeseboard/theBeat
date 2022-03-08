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
#include "librarymanager.h"

#include "libraryenumeratedirectoryjob.h"
#include "qtmultimedia/qtmultimediamediaitem.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QRandomGenerator>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>
#include <QVariant>
#include <playlist.h>
#include <statemanager.h>
#include <taglib/fileref.h>
#include <tjobmanager.h>

struct LibraryManagerPrivate {
        bool available = false;
        int isProcessing = 0;
};

LibraryManager::LibraryManager(QObject* parent) :
    QObject(parent) {
    d = new LibraryManagerPrivate();

    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir::root().mkpath(dataPath);
    QString dbPath = QDir(dataPath).absoluteFilePath("library.db");
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    if (!db.isValid()) return;
    db.setDatabaseName(dbPath);
    if (!db.open()) return;

    db.exec("PRAGMA foreign_keys = ON");
    db.exec("PRAGMA journal_mode = WAL");

    // Initialise the tables
    QStringList tables = db.tables();
    db.exec("CREATE TABLE IF NOT EXISTS version(version INTEGER)");

    int version = -1;
    QSqlQuery versionQuery("SELECT version FROM version");
    if (versionQuery.next()) {
        version = versionQuery.value("version").toInt();
    }

    if (version == -1) {
        // Initialise a new database; this is the first time we're running theBeat
        db.exec("CREATE TABLE tracks(id INTEGER PRIMARY KEY, path TEXT UNIQUE, title TEXT, artist TEXT, album TEXT, duration INTEGER, trackNumber INTEGER)");
        db.exec("CREATE TABLE blacklist(path TEXT PRIMARY KEY)");
        db.exec("CREATE TABLE playlists(id INTEGER PRIMARY KEY, name TEXT UNIQUE)");
        db.exec("CREATE TABLE playlistTracks(playlistid INTEGER REFERENCES playlists(id) ON DELETE CASCADE, trackid INTEGER REFERENCES tracks(id) ON DELETE CASCADE ON UPDATE CASCADE, sort INTEGER, CONSTRAINT playlistTracks_pk PRIMARY KEY(playlistid, trackid, sort))");
        db.exec("INSERT INTO version(version) VALUES(1)");

        // Also add Silly to the playlist
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
        StateManager::instance()->playlist()->addItem(new QtMultimediaMediaItem(QUrl("qrc:/resources/Silly.mp3")));
#else
        StateManager::instance()->playlist()->addItem(new QtMultimediaMediaItem(QUrl("qrc:/resources/Silly.ogg")));
#endif
        StateManager::instance()->playlist()->pause();
    }

    if (version <= 1) {
        // Upgrade to database version 2
        db.exec("CREATE TABLE playTime(id INTEGER PRIMARY KEY, trackId INTEGER REFERENCES tracks(id) ON DELETE CASCADE ON UPDATE CASCADE, date INTEGER)");
        db.exec("DELETE FROM version");
        db.exec("INSERT INTO version(version) VALUES(2)");
    }

    if (version <= 2) {
        // This is the current database version
    }

    // Enumerate the Music directory
    QTimer::singleShot(0, [=] {
        QStringList musicDirectories = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
        for (QString dir : musicDirectories) {
            this->enumerateDirectory(dir, false, false);
        }
    });
}

LibraryManager::~LibraryManager() {
}

LibraryManager* LibraryManager::instance() {
    static LibraryManager* mgr = new LibraryManager();
    return mgr;
}

void LibraryManager::enumerateDirectory(QString path, bool ignoreBlacklist, bool isUserAction) {
    d->isProcessing++;
    emit isProcessingChanged();

    LibraryEnumerateDirectoryJob* job = new LibraryEnumerateDirectoryJob(path, ignoreBlacklist, isUserAction);
    connect(job, &LibraryEnumerateDirectoryJob::stateChanged, this, [=](tJob::State state) {
        if (state == tJob::Finished) {
            d->isProcessing--;
            emit isProcessingChanged();

            emit libraryChanged();
        }
    });
    tJobManager::trackJob(job);
}

void LibraryManager::addTrack(QString path, bool updateOnly) {
    if (!updateOnly) {
        // Remove the path from the blacklist if it exists
        QSqlQuery blacklistQuery;
        blacklistQuery.prepare("DELETE FROM blacklist WHERE path=:path");
        blacklistQuery.bindValue(":path", path);
        blacklistQuery.exec();
    }

#ifdef Q_OS_WIN
    TagLib::FileRef file(reinterpret_cast<const wchar_t*>(path.constData()));
#else
    TagLib::FileRef file(path.toUtf8());
#endif
    TagLib::Tag* tag = file.tag();
    TagLib::AudioProperties* audioProperties = file.audioProperties();

    if (!tag) return;

    // TODO: make this less... ugly

    QVariantList paths, titles, artists, albums, durations, trackNumbers;

    paths.append(path);
    titles.append(tag->title().isNull() || tag->title().isEmpty() ? QFileInfo(path).baseName() : QString::fromStdString(tag->title().to8Bit(true)));
    artists.append(tag->artist().isNull() ? QVariant(QVariant::String) : QString::fromStdString(tag->artist().to8Bit(true)));
    albums.append(tag->album().isNull() ? QVariant(QVariant::String) : QString::fromStdString(tag->album().to8Bit(true)));
    durations.append(audioProperties->length() * 1000);
    trackNumbers.append(tag->track());

    QSqlQuery q;
    if (updateOnly) {
        q.prepare("UPDATE tracks SET title=:titleupd, artist=:artistupd, album=:albumupd, duration=:durationupd, trackNumber=:tracknumberupd WHERE path=:path");
    } else {
        q.prepare("INSERT INTO tracks(path, title, artist, album, duration, trackNumber) VALUES(:path, :title, :artist, :album, :duration, :tracknumber) ON CONFLICT (path) DO "
                  "UPDATE SET title=:titleupd, artist=:artistupd, album=:albumupd, duration=:durationupd, trackNumber=:tracknumberupd");
        q.bindValue(":title", titles);
        q.bindValue(":artist", artists);
        q.bindValue(":album", albums);
        q.bindValue(":duration", durations);
        q.bindValue(":tracknumber", trackNumbers);
    }
    q.bindValue(":path", paths);
    q.bindValue(":titleupd", titles);
    q.bindValue(":artistupd", artists);
    q.bindValue(":albumupd", albums);
    q.bindValue(":durationupd", durations);
    q.bindValue(":tracknumberupd", trackNumbers);
    q.execBatch();

    emit libraryChanged();
}

void LibraryManager::removeTrack(QString path) {
    TemporaryDatabase db;
    QSqlQuery q(db.db);
    q.prepare("DELETE FROM tracks WHERE path=:path");
    q.bindValue(":path", path);
    q.exec();

    emit libraryChanged();
}

void LibraryManager::blacklistTrack(QString path) {
    removeTrack(path);

    TemporaryDatabase db;
    QSqlQuery q(db.db);
    q.prepare("INSERT INTO blacklist(path) VALUES(:path)");
    q.bindValue(":path", path);
    q.exec();
}

void LibraryManager::relocateTrack(QString oldPath, QString newPath) {
    TemporaryDatabase db;
    QSqlQuery q(db.db);
    q.prepare("UPDATE OR REPLACE tracks SET path=:newpath WHERE path=:oldpath");
    q.bindValue(":oldpath", oldPath);
    q.bindValue(":newpath", newPath);
    q.exec();

    emit libraryChanged();
}

void LibraryManager::bumpTrackPlayCount(QString path) {
    TemporaryDatabase db;
    int trackId = this->trackId(path);
    if (trackId == -1) return;

    QSqlQuery q(db.db);
    q.prepare("INSERT INTO playTime(trackId, date) VALUES(:id, :date)");
    q.bindValue(":id", trackId);
    q.bindValue(":date", QDateTime::currentDateTime().toMSecsSinceEpoch());
    q.exec();
}

int LibraryManager::trackPlayCount(QString path) {
    TemporaryDatabase db;
    QSqlQuery q(db.db);
    q.prepare("SELECT COUNT(*) AS count FROM tracks NATURAL JOIN playTime WHERE tracks.path=:path AND playTime.date>:fromDate");
    q.bindValue(":path", path);
    q.bindValue(":fromDate", QDateTime::currentDateTime().addMonths(-1).toMSecsSinceEpoch());
    q.exec();

    if (q.next()) {
        return q.value("count").toInt();
    } else {
        return 0;
    }
}

LibraryModel* LibraryManager::allTracks() {
    LibraryModel* model = new LibraryModel();
    model->setQuery("SELECT * FROM tracks ORDER BY LOWER(title) ASC", model->database());
    return model;
}

LibraryModel* LibraryManager::searchTracks(QString query) {
    LibraryModel* model = new LibraryModel();
    QSqlQuery q(model->database());
    q.prepare("SELECT * FROM tracks WHERE title LIKE '%'||:query||'%' ORDER BY LOWER(title) ASC");
    q.bindValue(":query", query);
    q.exec();

    model->setQuery(q);
    return model;
}

int LibraryManager::countTracks() {
    TemporaryDatabase db;
    QSqlQuery q("SELECT COUNT(*) FROM tracks", db.db);
    q.next();

    return q.value(0).toInt();
}

QStringList LibraryManager::artists() {
    TemporaryDatabase db;
    QStringList artists;
    QSqlQuery q("SELECT DISTINCT artist FROM tracks WHERE artist IS NOT NULL AND artist != '' ORDER BY LOWER(artist) ASC", db.db);
    while (q.next()) {
        artists.append(q.value("artist").toString());
    }
    return artists;
}

QStringList LibraryManager::albums() {
    TemporaryDatabase db;
    QStringList albums;
    QSqlQuery q("SELECT DISTINCT album FROM tracks WHERE album IS NOT NULL AND album != '' ORDER BY LOWER(album) ASC", db.db);
    while (q.next()) {
        albums.append(q.value("album").toString());
    }
    return albums;
}

LibraryModel* LibraryManager::tracksByArtist(QString artist) {
    LibraryModel* model = new LibraryModel();
    QSqlQuery q(model->database());
    q.prepare("SELECT * FROM tracks WHERE artist=:artist ORDER BY LOWER(title) ASC");
    q.bindValue(":artist", artist);
    q.exec();

    model->setQuery(q);
    return model;
}

LibraryModel* LibraryManager::tracksByAlbum(QString album) {
    LibraryModel* model = new LibraryModel();
    QSqlQuery q(model->database());
    q.prepare("SELECT * FROM tracks WHERE album=:album ORDER BY trackNumber ASC");
    q.bindValue(":album", album);
    q.exec();

    model->setQuery(q);
    return model;
}

int LibraryManager::createPlaylist(QString playlistName) {
    int lastInsertId;

    {
        TemporaryDatabase db;
        QSqlQuery q(db.db);
        q.prepare("INSERT INTO playlists(name) VALUES(:name)");
        q.bindValue(":name", playlistName);
        q.exec();

        lastInsertId = q.lastInsertId().toInt();
    }

    emit playlistsChanged();

    return lastInsertId;
}

QList<QPair<int, QString>> LibraryManager::playlists() {
    TemporaryDatabase db;
    QSqlQuery q("SELECT * FROM playlists ORDER BY LOWER(name) ASC", db.db);
    QList<QPair<int, QString>> playlists;
    while (q.next()) {
        playlists.append({q.value("id").toInt(), q.value("name").toString()});
    }
    return playlists;
}

void LibraryManager::removePlaylist(int playlist) {
    {
        TemporaryDatabase db;
        QSqlQuery q(db.db);
        q.prepare("DELETE FROM playlists WHERE id=:playlistid");
        q.bindValue(":playlistid", playlist);
        q.exec();
    }

    emit playlistsChanged();
}

void LibraryManager::renamePlaylist(int playlist, QString name) {
    {
        TemporaryDatabase db;
        QSqlQuery q(db.db);
        q.prepare("UPDATE playlists SET name=:name WHERE id=:playlistid");
        q.bindValue(":name", name);
        q.bindValue(":playlistid", playlist);
        q.exec();
    }

    emit playlistsChanged();
}

void LibraryManager::addTrackToPlaylist(int playlist, QString path) {
    {
        TemporaryDatabase db;
        QSqlQuery q(db.db);
        q.prepare("INSERT INTO playlistTracks(playlistid, trackid, sort) VALUES(:playlistid, (SELECT id FROM tracks WHERE path=:path), (SELECT COUNT(*) FROM playlistTracks WHERE playlistid=:playlistidcount) * 2)");
        q.bindValue(":playlistid", playlist);
        q.bindValue(":playlistidcount", playlist);
        q.bindValue(":path", path);
        q.exec();
    }

    emit playlistChanged(playlist);
}

void LibraryManager::removeTrackFromPlaylist(int playlist, int sort) {
    {
        TemporaryDatabase db;
        QSqlQuery q(db.db);
        q.prepare("DELETE FROM playlistTracks WHERE playlistid=:playlistid AND sort=:sort");
        q.bindValue(":playlistid", playlist);
        q.bindValue(":sort", sort);
        q.exec();
    }

    emit playlistChanged(playlist);
}

LibraryModel* LibraryManager::tracksByPlaylist(int playlist) {
    LibraryModel* model = new LibraryModel();
    QSqlQuery q(model->database());
    q.prepare("SELECT tracks.*, playlistTracks.sort FROM playlistTracks, tracks WHERE playlistTracks.trackid=tracks.id AND playlistTracks.playlistid=:playlist ORDER BY playlistTracks.sort ASC");
    q.bindValue(":playlist", playlist);
    q.exec();

    model->setQuery(q);
    return model;
}

void LibraryManager::normalisePlaylistSort(int playlist) {
    TemporaryDatabase db;
    QSqlQuery q(db.db);
    q.prepare("SELECT * FROM playlistTracks WHERE playlistid=:playlistid ORDER BY sort ASC");
    q.bindValue(":playlistid", playlist);
    q.exec();

    int i = 0;
    QList<QPair<int, int>> sorts;
    while (q.next()) {
        sorts.append({q.value("sort").toInt(), i * 2});
        i++;
    }

    for (QPair<int, int> sort : sorts) {
        if (sort.first != sort.second) {
            QSqlQuery q(db.db);
            q.prepare("UPDATE playlistTracks SET sort=:newSort WHERE sort=:oldSort AND playlistid=:playlistid");
            q.bindValue(":newSort", sort.second);
            q.bindValue(":oldSort", sort.first);
            q.bindValue(":playlistid", playlist);
            q.exec();
        }
    }

    db.db.transaction();
    db.db.commit();
}

LibraryModel* LibraryManager::smartPlaylist(SmartPlaylist smartPlaylist) {
    LibraryModel* model = new LibraryModel();
    switch (smartPlaylist) {
        case LibraryManager::Frequents:
            {
                QSqlQuery q(model->database());
                q.prepare("SELECT tracks.*, counts.count FROM tracks, (SELECT *, COUNT(*) AS count FROM playTime WHERE playTime.date>:fromDate GROUP BY playTime.trackId) AS counts WHERE counts.trackId=tracks.id ORDER BY counts.count DESC LIMIT 20");
                q.bindValue(":fromDate", QDateTime::currentDateTime().addMonths(-1).toMSecsSinceEpoch());
                q.exec();

                model->setQuery(q);
                break;
            }
        case LibraryManager::Random:
            model->setQuery("SELECT * FROM tracks ORDER BY random() DESC LIMIT 20", model->database());
            break;
        case LibraryManager::LastSmartPlaylist:
            break;
    }
    return model;
}

QString LibraryManager::smartPlaylistName(SmartPlaylist smartPlaylist) {
    switch (smartPlaylist) {
        case LibraryManager::Frequents:
            return tr("20 Most Played Tracks");
        case LibraryManager::Random:
            return tr("20 Random Tracks");
        default:
            return "";
    }
}

bool LibraryManager::isProcessing() {
    return d->isProcessing > 0;
}

void LibraryManager::erase() {
    QStringList connections = QSqlDatabase::connectionNames();
    for (QString connection : connections) {
        QSqlDatabase db = QSqlDatabase::database(connection);
        db.close();
        QSqlDatabase::removeDatabase(connection);
    }

    QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QFile::remove(dataDir.absoluteFilePath("library.db"));
    QFile::remove(dataDir.absoluteFilePath("library.db-shm"));
    QFile::remove(dataDir.absoluteFilePath("library.db-wal"));
}

int LibraryManager::trackId(QString path) {
    TemporaryDatabase db;
    QSqlQuery q(db.db);
    q.prepare("SELECT id FROM tracks WHERE tracks.path=:path");
    q.bindValue(":path", path);
    q.exec();

    if (q.next()) {
        return q.value("id").toInt();
    } else {
        return -1;
    }
}

TemporaryDatabase::TemporaryDatabase() {
    QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnpopqrstuvwxyz";
    for (int i = 0; i < 12; i++) {
        dbName.append(chars.at(QRandomGenerator::global()->bounded(chars.length())));
    }

    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir::root().mkpath(dataPath);
    QString dbPath = QDir(dataPath).absoluteFilePath("library.db");

    db = QSqlDatabase::addDatabase("QSQLITE", dbName);
    if (!db.isValid()) return;
    db.setDatabaseName(dbPath);
    if (!db.open()) return;

    db.exec("PRAGMA foreign_keys = ON");
    db.exec("PRAGMA journal_mode = WAL");
}

TemporaryDatabase::~TemporaryDatabase() {
    db.close();
    QSqlDatabase::removeDatabase(dbName);
}
