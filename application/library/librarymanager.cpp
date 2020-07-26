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
#include <tpromise.h>
#include <taglib/fileref.h>

struct LibraryManagerPrivate {
    bool available = false;
    int isProcessing = 0;
};

struct TemporaryDatabase {
    QString dbName;
    QSqlDatabase db;

    TemporaryDatabase() {
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
    }

    ~TemporaryDatabase() {
        db.close();
        QSqlDatabase::removeDatabase(dbName);
    }
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

    //Initialise the tables
    QStringList tables = db.tables();
    db.exec("CREATE TABLE tracks(id INTEGER PRIMARY KEY, path TEXT UNIQUE, title TEXT, artist TEXT, album TEXT, duration INTEGER, trackNumber INTEGER)");
    db.exec("CREATE TABLE blacklist(path TEXT PRIMARY KEY)");

    //Enumerate the Music directory
    QTimer::singleShot(0, [ = ] {
        QStringList musicDirectories = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
        for (QString dir : musicDirectories) {
            this->enumerateDirectory(dir);
        }
    });
}

LibraryManager* LibraryManager::instance() {
    static LibraryManager* mgr = new LibraryManager();
    return mgr;
}

void LibraryManager::enumerateDirectory(QString path) {
    d->isProcessing++;
    emit isProcessingChanged();

    QString blacklistedPaths;
    QSqlQuery blacklistQuery("SELECT * FROM blacklist");
    while (blacklistQuery.next()) {
        blacklistedPaths.append(blacklistQuery.value("path").toString());
    }

    QVariantList paths, titles, artists, albums, durations, trackNumbers;

    QDirIterator iterator(path, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        QString path = iterator.next();
        if (blacklistedPaths.contains(path)) continue;

#ifdef Q_OS_WIN
        TagLib::FileRef file(path.toUtf8().data());
#else
        TagLib::FileRef file(path.toUtf8());
#endif
        TagLib::Tag* tag = file.tag();
        TagLib::AudioProperties* audioProperties = file.audioProperties();

        if (!tag) continue;

        paths.append(path);
        titles.append(tag->title().isNull() || tag->title().isEmpty() ? QFileInfo(path).baseName() : QString::fromStdString(tag->title().to8Bit(true)));
        artists.append(tag->artist().isNull() ? QVariant(QVariant::String) : QString::fromStdString(tag->artist().to8Bit(true)));
        albums.append(tag->album().isNull() ? QVariant(QVariant::String) : QString::fromStdString(tag->album().to8Bit(true)));
        durations.append(audioProperties->lengthInMilliseconds());
        trackNumbers.append(tag->track());
    }

    tPromise<void>::runOnNewThread([ = ](tPromiseFunctions<void>::SuccessFunction res, tPromiseFunctions<void>::FailureFunction rej) {
        TemporaryDatabase db;

        db.db.exec("PRAGMA journal_mode = WAL");

        db.db.transaction();

        QSqlQuery q(db.db);
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

        db.db.commit();
        db.db.exec("PRAGMA wal_checkpoint(TRUNCATE)");

        res();
    })->then([ = ] {
        d->isProcessing--;
        emit isProcessingChanged();

        emit libraryChanged();
    });
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

    if (!tag) return;

    //TODO: make this less... ugly

    QVariantList paths, titles, artists, albums, durations, trackNumbers;

    paths.append(path);
    titles.append(tag->title().isNull() ? QFileInfo(path).baseName() : QString::fromStdString(tag->title().to8Bit(true)));
    artists.append(tag->artist().isNull() ? QVariant(QVariant::String) : QString::fromStdString(tag->artist().to8Bit(true)));
    albums.append(tag->album().isNull() ? QVariant(QVariant::String) : QString::fromStdString(tag->album().to8Bit(true)));
    durations.append(QVariant(QVariant::Int));
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
    model->setQuery(q);
    return model;
}

bool LibraryManager::isProcessing() {
    return d->isProcessing > 0;
}
