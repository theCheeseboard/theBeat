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
#include "libraryenumeratedirectoryjob.h"

#include "libraryenumeratedirectoryjobwidget.h"
#include <QSqlQuery>
#include <tnotification.h>
#include <tpromise.h>
#include <QDirIterator>
#include <fileref.h>
#include <tag.h>
#include <audioproperties.h>
#include "librarymanager.h"

struct LibraryEnumerateDirectoryJobPrivate {
    QString path;
    bool ignoreBlacklist;
    bool isUserAction;

    quint64 progress = 0;
    quint64 total = 0;

    static QMutex mutex;

    tJob::State state = tJob::Processing;
};

QMutex LibraryEnumerateDirectoryJobPrivate::mutex = QMutex();

LibraryEnumerateDirectoryJob::LibraryEnumerateDirectoryJob(QString path, bool ignoreBlacklist, bool isUserAction, QObject* parent) : tJob(parent) {
    d = new LibraryEnumerateDirectoryJobPrivate();
    d->path = path;
    d->ignoreBlacklist = ignoreBlacklist;
    d->isUserAction = isUserAction;

    performEnumeration();
}

LibraryEnumerateDirectoryJob::~LibraryEnumerateDirectoryJob() {

}

void LibraryEnumerateDirectoryJob::performEnumeration() {
    QString blacklistedPaths;
    QSqlQuery blacklistQuery("SELECT * FROM blacklist");
    while (blacklistQuery.next()) {
        blacklistedPaths.append(blacklistQuery.value("path").toString());
    }

    tPromise<int>::runOnNewThread([ = ](tPromiseFunctions<int>::SuccessFunction res, tPromiseFunctions<int>::FailureFunction rej) {
        Q_UNUSED(rej)

        //Lock the mutex
        d->mutex.lock();

        //Figure out the number of files to track
        quint64 totalFiles = 0;
        QDirIterator trackerIterator(d->path, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (trackerIterator.hasNext()) {
            trackerIterator.next();
            totalFiles++;
        }

        d->total = totalFiles * 2;
        emit totalProgressChanged(d->total);

        QVariantList paths, titles, artists, albums, durations, trackNumbers;
        QDirIterator iterator(d->path, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (iterator.hasNext()) {
            QString path = iterator.next();
            if (blacklistedPaths.contains(path) && !d->ignoreBlacklist) continue;

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
            durations.append(audioProperties->length() * 1000);
            trackNumbers.append(tag->track());

            d->progress++;
            emit progressChanged(d->progress);
        }

        TemporaryDatabase db;

        db.db.transaction();

        QSqlQuery blacklistQ;
        blacklistQ.prepare("DELETE FROM blacklist WHERE path=:path");
        blacklistQ.bindValue(":path", paths);
        blacklistQ.execBatch();

        int rowsAffected = 0;

        for (int i = 0; i < paths.count(); i += 50) {
            QSqlQuery q(db.db);
            q.prepare("INSERT INTO tracks(path, title, artist, album, duration, trackNumber) VALUES(:path, :title, :artist, :album, :duration, :tracknumber) ON CONFLICT (path) DO "
                "UPDATE SET title=:titleupd, artist=:artistupd, album=:albumupd, duration=:durationupd, trackNumber=:tracknumberupd");
            q.bindValue(":path", paths.mid(i, 50));
            q.bindValue(":title", titles.mid(i, 50));
            q.bindValue(":artist", artists.mid(i, 50));
            q.bindValue(":album", albums.mid(i, 50));
            q.bindValue(":duration", durations.mid(i, 50));
            q.bindValue(":tracknumber", trackNumbers.mid(i, 50));
            q.bindValue(":titleupd", titles.mid(i, 50));
            q.bindValue(":artistupd", artists.mid(i, 50));
            q.bindValue(":albumupd", albums.mid(i, 50));
            q.bindValue(":durationupd", durations.mid(i, 50));
            q.bindValue(":tracknumberupd", trackNumbers.mid(i, 50));
            q.execBatch();

            rowsAffected += q.numRowsAffected();

            d->progress += 50;
            emit progressChanged(d->progress);
        }

        db.db.commit();

        d->mutex.unlock();

        res(rowsAffected);
    })->then([ = ](int result) {
        if (d->isUserAction) {
            tNotification* notification = new tNotification();
            notification->setSummary(tr("Folder Added"));
            notification->setText(tr("%n tracks added/updated", nullptr, result));
            notification->post();
        }

        d->state = tJob::Finished;
        emit stateChanged(tJob::Finished);
    });
}

quint64 LibraryEnumerateDirectoryJob::progress() {
    return 0;
}

quint64 LibraryEnumerateDirectoryJob::totalProgress() {
    return 0;
}

tJob::State LibraryEnumerateDirectoryJob::state() {
    return d->state;
}

QWidget* LibraryEnumerateDirectoryJob::makeProgressWidget() {
    return new LibraryEnumerateDirectoryJobWidget(this);
}

bool LibraryEnumerateDirectoryJob::isTransient() {
    return true;
}
