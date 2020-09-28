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

#include <QDir>
#include <QSqlDatabase>
#include <QStandardPaths>
#include <QSqlQuery>
#include <QSqlError>
#include <QDirIterator>
#include <QVariant>
#include <QDebug>
#include <QRandomGenerator>
#include <statemanager.h>
#include <playlist.h>
#include "qtmultimedia/qtmultimediamediaitem.h"
#include <tpromise.h>
#include <taglib/fileref.h>
#include "libraryenumeratedirectoryjob.h"
#include <tjobmanager.h>

struct LibraryManagerPrivate {
    bool available = false;
    int isProcessing = 0;
};

LibraryManager::LibraryManager(QObject* parent) : QObject(parent) {
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

    //Initialise the tables
    QStringList tables = db.tables();
    db.exec("CREATE TABLE IF NOT EXISTS version(version INTEGER)");

    int version = -1;
    QSqlQuery versionQuery("SELECT version FROM version");
    if (versionQuery.next()) {
        version = versionQuery.value("version").toInt();
    }

    if (version == -1) {
        //Initialise a new database; this is the first time we're running theBeat
        db.exec("CREATE TABLE tracks(id INTEGER PRIMARY KEY, path TEXT UNIQUE, title TEXT, artist TEXT, album TEXT, duration INTEGER, trackNumber INTEGER)");
        db.exec("CREATE TABLE blacklist(path TEXT PRIMARY KEY)");
        db.exec("CREATE TABLE playlists(id INTEGER PRIMARY KEY, name TEXT UNIQUE)");
        db.exec("CREATE TABLE playlistTracks(playlistid INTEGER REFERENCES playlists(id) ON DELETE CASCADE, trackid INTEGER REFERENCES tracks(id) ON DELETE CASCADE ON UPDATE CASCADE, sort INTEGER, CONSTRAINT playlistTracks_pk PRIMARY KEY(playlistid, trackid, sort))");
        db.exec("INSERT INTO version(version) VALUES(1)");

        //Also add Silly to the playlist
#ifdef Q_OS_WIN
        StateManager::instance()->playlist()->addItem(new QtMultimediaMediaItem(QUrl("qrc:/resources/Silly.mp3")));
#else
        StateManager::instance()->playlist()->addItem(new QtMultimediaMediaItem(QUrl("qrc:/resources/Silly.ogg")));
#endif
        StateManager::instance()->playlist()->pause();
    } else if (version == 1) {
        //This is the current database version
    }

    //Enumerate the Music directory
    QTimer::singleShot(0, [ = ] {
        QStringList musicDirectories = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
        for (QString dir : musicDirectories) {
            this->enumerateDirectory(dir, false, false);
        }
    });
}

LibraryManager* LibraryManager::instance() {
    static LibraryManager* mgr = new LibraryManager();
    return mgr;
}

void LibraryManager::enumerateDirectory(QString path, bool ignoreBlacklist, bool isUserAction) {
    d->isProcessing++;
    emit isProcessingChanged();

    LibraryEnumerateDirectoryJob* job = new LibraryEnumerateDirectoryJob(path, ignoreBlacklist, isUserAction);
    connect(job, &LibraryEnumerateDirectoryJob::stateChanged, this, [ = ](tJob::State state) {
        if (state == tJob::Finished) {
            d->isProcessing--;
            emit isProcessingChanged();

            emit libraryChanged();
        }
    });
    tJobManager::trackJob(job);
}

void LibraryManager::addTrack(QString path) {
    //Remove the path from the blacklist if it exists
    QSqlQuery blacklistQuery;
    blacklistQuery.prepare("DELETE FROM blacklist WHERE path=:path");
    blacklistQuery.bindValue(":path", path);
    blacklistQuery.exec();

#ifdef Q_OS_WIN
    TagLib::FileRef file(path.toUtf8().data());
#else
    TagLib::FileRef file(path.toUtf8());
#endif
    TagLib::Tag* tag = file.tag();
    TagLib::AudioProperties* audioProperties = file.audioProperties();

    if (!tag) return;

    //TODO: make this less... ugly

    QVariantList paths, titles, artists, albums, durations, trackNumbers;

    paths.append(path);
    titles.append(tag->title().isNull() || tag->title().isEmpty() ? QFileInfo(path).baseName() : QString::fromStdString(tag->title().to8Bit(true)));
    artists.append(tag->artist().isNull() ? QVariant(QVariant::String) : QString::fromStdString(tag->artist().to8Bit(true)));
    albums.append(tag->album().isNull() ? QVariant(QVariant::String) : QString::fromStdString(tag->album().to8Bit(true)));
    durations.append(audioProperties->length() * 1000);
    trackNumbers.append(tag->track());

    QSqlQuery q;
    q.prepare("INSERT INTO tracks(path, title, artist, album, duration, trackNumber) VALUES(:path, :title, :artist, :album, :duration, :tracknumber) ON CONFLICT (path) DO "
        "UPDATE SET title=:titleupd, artist=:artistupd, album=:albumupd, duration=:durationupd, trackNumber=:tracknumberupd");
    q.bindValue(":path", paths);
    q.bindValue(":title", titles);
    q.bindValue(":artist", artists);
    q.bindValue(":album", albums);
    q.bindValue(":duration", durations);
    q.bindValue(":tracknumber", trackNumbers);
    q.bindValue(":titleupd", titles);
    q.bindValue(":artistupd", artists);
    q.bindValue(":albumupd", albums);
    q.bindValue(":durationupd", durations);
    q.bindValue(":tracknumberupd", trackNumbers);
    q.execBatch();

    emit libraryChanged();
}

void LibraryManager::removeTrack(QString path) {
    QSqlQuery q;
    q.prepare("DELETE FROM tracks WHERE path=:path");
    q.bindValue(":path", path);
    q.exec();

    emit libraryChanged();
}

void LibraryManager::blacklistTrack(QString path) {
    removeTrack(path);

    QSqlQuery q;
    q.prepare("INSERT INTO blacklist(path) VALUES(:path)");
    q.bindValue(":path", path);
    q.exec();
}

void LibraryManager::relocateTrack(QString oldPath, QString newPath) {
    QSqlQuery q;
    q.prepare("UPDATE OR REPLACE tracks SET path=:newpath WHERE path=:oldpath");
    q.bindValue(":oldpath", oldPath);
    q.bindValue(":newpath", newPath);
    q.exec();

    emit libraryChanged();
}

LibraryModel* LibraryManager::allTracks() {
    LibraryModel* model = new LibraryModel();
    model->setQuery("SELECT * FROM tracks ORDER BY LOWER(title) ASC");
    return model;
}

LibraryModel* LibraryManager::searchTracks(QString query) {
    QSqlQuery q;
    q.prepare("SELECT * FROM tracks WHERE title LIKE '%'||:query||'%' ORDER BY LOWER(title) ASC");
    q.bindValue(":query", query);
    q.exec();

    LibraryModel* model = new LibraryModel();
    model->setQuery(q);
    return model;
}

int LibraryManager::countTracks() {
    QSqlQuery q("SELECT COUNT(*) FROM tracks");
    q.next();

    return q.value(0).toInt();
}

QStringList LibraryManager::artists() {
    QStringList artists;
    QSqlQuery q("SELECT DISTINCT artist FROM tracks WHERE artist IS NOT NULL AND artist != '' ORDER BY LOWER(artist) ASC");
    while (q.next()) {
        artists.append(q.value("artist").toString());
    }
    return artists;
}

QStringList LibraryManager::albums() {
    QStringList albums;
    QSqlQuery q("SELECT DISTINCT album FROM tracks WHERE album IS NOT NULL AND album != '' ORDER BY LOWER(album) ASC");
    while (q.next()) {
        albums.append(q.value("album").toString());
    }
    return albums;
}

LibraryModel* LibraryManager::tracksByArtist(QString artist) {
    QSqlQuery q;
    q.prepare("SELECT * FROM tracks WHERE artist=:artist ORDER BY LOWER(title) ASC");
    q.bindValue(":artist", artist);
    q.exec();

    LibraryModel* model = new LibraryModel();
    model->setQuery(q);
    return model;
}

LibraryModel* LibraryManager::tracksByAlbum(QString album) {
    QSqlQuery q;
    q.prepare("SELECT * FROM tracks WHERE album=:album ORDER BY trackNumber ASC");
    q.bindValue(":album", album);
    q.exec();

    LibraryModel* model = new LibraryModel();
    model->setQuery(q);//        if (connection != QSqlDatabase::defaultConnection) {

    return model;
}

void LibraryManager::createPlaylist(QString playlistName) {
    QSqlQuery q;
    q.prepare("INSERT INTO playlists(name) VALUES(:name)");
    q.bindValue(":name", playlistName);
    q.exec();

    emit playlistsChanged();
}

QList<QPair<int, QString>> LibraryManager::playlists() {
    QSqlQuery q("SELECT * FROM playlists ORDER BY LOWER(name) ASC");
    QList<QPair<int, QString>> playlists;
    while (q.next()) {
        playlists.append({q.value("id").toInt(), q.value("name").toString()});
    }
    return playlists;
}

void LibraryManager::addTrackToPlaylist(int playlist, QString path) {
    QSqlQuery q;
    q.prepare("INSERT INTO playlistTracks(playlistid, trackid, sort) VALUES(:playlistid, (SELECT id FROM tracks WHERE path=:path), (SELECT COUNT(*) FROM playlistTracks WHERE playlistid=:playlistidcount))");
    q.bindValue(":playlistid", playlist);
    q.bindValue(":playlistidcount", playlist);
    q.bindValue(":path", path);
    q.exec();

    emit playlistChanged(playlist);
}

LibraryModel* LibraryManager::tracksByPlaylist(int playlist) {
    QSqlQuery q;
    q.prepare("SELECT tracks.* FROM playlistTracks, tracks WHERE playlistTracks.trackid=tracks.id AND playlistTracks.playlistid=:playlist ORDER BY playlistTracks.sort ASC");
    q.bindValue(":playlist", playlist);
    q.exec();

    LibraryModel* model = new LibraryModel();
    model->setQuery(q);
    return model;
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
    db.exec("PRAGMA wal_checkpoint(TRUNCATE)");
    db.close();
    QSqlDatabase::removeDatabase(dbName);
}
