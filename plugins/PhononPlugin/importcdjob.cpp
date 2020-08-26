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
#include "importcdjob.h"

#include <QProcess>
#include <QTemporaryDir>
#include "importcdjobwidget.h"
#include <cdio/paranoia/paranoia.h>
#include <tnotification.h>

#include <taglib/fileref.h>
#include <taglib/tpropertymap.h>

struct ImportCdJobPrivate {
    tJob::State state = tJob::Processing;
    quint64 progress;

    QTemporaryDir workDir;
    QDir outputDir;

    QString blockDevice;
    QList<TrackInfoPtr> trackInfo;

    QString description;

    int nextTrack = 0;
    int totalLength = 0;
    int readSpeed;

    bool canCancel = true;
    bool cancelNext = false;
};

ImportCdJob::ImportCdJob(QString blockDevice, QList<TrackInfoPtr> trackInfo, QString outputDir, int readSpeed, QObject* parent) : tJob(parent) {
    d = new ImportCdJobPrivate();
    d->blockDevice = blockDevice;
    d->trackInfo = trackInfo;

    d->outputDir.setPath(outputDir);
    if (!d->outputDir.exists()) {
        QDir::root().mkpath(outputDir);
    }

    d->description = tr("Preparing to import");
    performNextAction();
}

ImportCdJob::~ImportCdJob() {
    delete d;
}

QString ImportCdJob::description() {
    return d->description;
}

bool ImportCdJob::canCancel() {
    return d->canCancel;
}

void ImportCdJob::cancel() {
    d->canCancel = false;
    d->cancelNext = true;

    d->description = tr("Cancelling");
    emit descriptionChanged(d->description);
}

void ImportCdJob::performNextAction() {
    if (d->cancelNext) {
        d->state = Failed;
        emit stateChanged(Failed);

        d->description = tr("Cancelled");
        emit descriptionChanged(d->description);
        return;
    }

    if (d->nextTrack == 0) {
        //Query cdparanoia
        QProcess* process = new QProcess();
        process->setProcessChannelMode(QProcess::MergedChannels);
        process->setWorkingDirectory(d->workDir.path());
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [ = ](int exitCode, QProcess::ExitStatus status) {
            if (exitCode != 0) {
                fail(tr("Couldn't query disc info"));
                return;
            }

            QString line;
            while (!(line = process->readLine()).startsWith("TOTAL")); //do nothing

            QString totalLength = line.split(" ", Qt::SkipEmptyParts).at(1);
            d->totalLength = totalLength.toInt();
            emit totalProgressChanged(d->totalLength);

            d->nextTrack = 1;
            performNextAction();
            process->deleteLater();
        });
        process->start("cdparanoia", {"-Q"});
    } else if (d->nextTrack <= d->trackInfo.count()) {
        //Extract the audio

        TrackInfoPtr trackInfo = d->trackInfo.at(d->nextTrack - 1);
        d->description = tr("Importing %1").arg(trackInfo->title());
        emit descriptionChanged(d->description);

        QProcess* process = new QProcess();
        process->setProcessChannelMode(QProcess::MergedChannels);
        process->setWorkingDirectory(d->workDir.path());
        connect(process, &QProcess::readyRead, this, [ = ] {
            while (process->canReadLine()) {
                QString line = process->readLine();
                QStringList parts = line.split(" ", Qt::SkipEmptyParts);
                if (parts.first() != "##:") continue;
                if (parts.at(2) != "[read]") continue;
                QString inpos = parts.last().trimmed();

                bool ok;
                int currentInpos = inpos.toInt(&ok);
                if (ok) {
                    d->progress = currentInpos / CD_FRAMEWORDS;
                    emit progressChanged(d->progress);
                }
            }
        });
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [ = ](int exitCode, QProcess::ExitStatus status) {
            if (exitCode != 0) {
                fail(tr("Couldn't import track"));
                return;
            }

            //Invoke ffmpeg to convert the audio files
            QString endFile = d->outputDir.filePath(QStringLiteral("%1 %2.%3").arg(trackInfo->track() + 1, 2, 10, QLatin1Char('0')).arg(trackInfo->title()).arg("ogg"));
            QProcess* ffmpeg = new QProcess();
            ffmpeg->setWorkingDirectory(d->workDir.path());
            connect(ffmpeg, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [ = ](int exitCode, QProcess::ExitStatus status) {
                //TODO: Tag the file
                TagLib::PropertyMap map;
                map.insert("TITLE", TagLib::StringList(trackInfo->title().toStdString().data()));

                TagLib::StringList artists;
                for (QString artist : trackInfo->artist()) {
                    artists.append(artist.toStdString().data());
                }
                map.insert("ARTIST", artists);
                map.insert("ALBUM", TagLib::StringList(trackInfo->album().toStdString().data()));
                map.insert("TRACKNUMBER", TagLib::StringList(QString::number(trackInfo->track() + 1).toStdString().data()));

                TagLib::FileRef file(endFile.toStdString().data());
                file.file()->setProperties(map);
                file.save();

                //TODO: Add the file to the library

                d->nextTrack++;
                performNextAction();
                ffmpeg->deleteLater();
            });
            ffmpeg->start("ffmpeg", {"-i", "track.wav", endFile});

            process->deleteLater();
        });

        QStringList paranoiaArgs = {"-we"};
        if (d->readSpeed != -1) {
            paranoiaArgs.append({"-S", QString::number(d->readSpeed)});
        }
        paranoiaArgs.append({"--force-cdrom-device", "/dev/" + d->blockDevice, "--", QString::number(d->nextTrack), "track.wav"});
        process->start("cdparanoia", paranoiaArgs);
    } else {
        d->state = Finished;
        emit stateChanged(Finished);

        d->progress = d->totalLength;
        emit progressChanged(d->progress);

        d->description = tr("Import Successful");
        emit descriptionChanged(d->description);

        d->workDir.remove();

        QString album = tr("CD");
        if (!d->trackInfo.isEmpty()) {
            album = d->trackInfo.first()->album();
        }

        tNotification* notification = new tNotification(tr("Import Successful"), tr("Imported \"%1\" successfully").arg(album));
        notification->post();
    }
}

void ImportCdJob::fail(QString description) {
    d->state = Failed;
    emit stateChanged(Failed);

    d->description = description;
    emit descriptionChanged(d->description);

    d->workDir.remove();

    QString album = tr("CD");
    if (!d->trackInfo.isEmpty()) {
        album = d->trackInfo.first()->album();
    }

    //Fire a notification
    tNotification* notification = new tNotification(tr("Import Failure"), tr("Failed to import \"%1\"").arg(album));
    notification->post();
}

quint64 ImportCdJob::progress() {
    return d->progress;
}

quint64 ImportCdJob::totalProgress() {
    return d->totalLength;
}

QWidget* ImportCdJob::makeProgressWidget() {
    return new ImportCdJobWidget(this);
}

tJob::State ImportCdJob::state() {
    return d->state;
}
