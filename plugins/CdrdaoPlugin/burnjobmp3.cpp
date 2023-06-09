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
#include "burnjobmp3.h"
#include "burnjobwidget.h"

#include <QProcess>
#include <QTemporaryDir>
#include <QTextStream>
#include <tlogger.h>
#include <tnotification.h>

#include <DriveObjects/blockinterface.h>
#include <DriveObjects/diskobject.h>
#include <DriveObjects/driveinterface.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>

struct BurnJobMp3Private {
        enum BurnState {
            LeadIn,
            Tracks,
            LeadOut
        };

        QStringList sourceFiles;
        DiskObject* diskObject;
        QString albumTitle;
        int nextItem = 0;

        BurnJobMp3::State state = BurnJobMp3::Processing;
        BurnState burnState = LeadIn;

        QString description;
        bool canCancel;
        bool cancelNext;
        bool warnCancel = false;
        QProcess* daoProcess = nullptr;

        quint64 progress;
        quint64 maxProgress;

        QByteArray processBuffer;

        QTemporaryDir workDir;
};

BurnJobMp3::BurnJobMp3(QStringList files, DiskObject* diskObject, QString albumTitle, QObject* parent) :
    AbstractBurnJob(parent) {
    d = new BurnJobMp3Private();
    d->sourceFiles = files;
    d->diskObject = diskObject;
    d->albumTitle = albumTitle;

    QDir(d->workDir.path()).mkdir("cd");

    d->description = tr("Preparing to burn");
}

BurnJobMp3::~BurnJobMp3() {
    delete d;
}

QCoro::Task<> BurnJobMp3::start() {
    performNextAction();
    co_return;
}

QString BurnJobMp3::description() {
    return d->description;
}

bool BurnJobMp3::canCancel() {
    return d->canCancel;
}

bool BurnJobMp3::warnCancel() {
    return d->warnCancel;
}

void BurnJobMp3::cancel() {
    d->cancelNext = true;
    d->canCancel = false;
    emit canCancelChanged(false);

    if (d->daoProcess) {
        d->daoProcess->terminate();
    }
}

QCoro::Task<> BurnJobMp3::performNextAction() {
    if (d->nextItem < d->sourceFiles.count()) {
        TagLib::FileRef file(d->sourceFiles.at(d->nextItem).toStdString().data());

        QString trackName;
        if (file.tag()->title().isEmpty()) {
            trackName = QFileInfo(d->sourceFiles.at(d->nextItem)).baseName();
        } else {
            trackName = QString::fromStdString(file.tag()->title().to8Bit());
        }

        d->description = tr("Preparing %1 to be burned").arg(QLocale().quoteString(trackName));
        emit descriptionChanged(d->description);

        // Transcode the file with ffmpeg
        QString sourceFile = d->sourceFiles.at(d->nextItem);
        QProcess* ffmpeg = new QProcess();
        ffmpeg->setWorkingDirectory(d->workDir.path());
        connect(ffmpeg, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, ffmpeg](int exitCode, QProcess::ExitStatus status) {
            if (d->cancelNext) {
                fail(tr("Cancelled"));
                return;
            }

            if (exitCode != 0) {
                fail(tr("Couldn't transcode track"));
                return;
            }

            d->nextItem++;
            performNextAction();
            ffmpeg->deleteLater();
        });

        // Name the file starting at track 1
        QStringList ffmpegArgs = {"-i", sourceFile, d->workDir.filePath(QStringLiteral("cd/%1 %2.mp3").arg(d->nextItem + 1, 2, 10, QLatin1Char('0')).arg(trackName))};
        tDebug("cdrdao") << "Calling ffmpeg with arguments" << ffmpegArgs;
        ffmpeg->start("ffmpeg", ffmpegArgs);

        // Make this part the first half of the total progress
        d->maxProgress = d->sourceFiles.count() * 2;
        d->progress = d->nextItem;
        emit totalProgressChanged(d->maxProgress);
        emit progressChanged(d->progress);

        d->canCancel = true;
        emit canCancelChanged(d->canCancel);
    } else if (d->nextItem == d->sourceFiles.count()) {
        // Generate ISO file
        d->description = tr("Generating Disc Image");
        emit descriptionChanged(d->description);

        // Call mkisofs to create an ISO file
        QProcess* process = new QProcess();
        process->setProcessChannelMode(QProcess::MergedChannels);
        process->setWorkingDirectory(d->workDir.path());
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, process](int exitCode, QProcess::ExitStatus status) {
            if (exitCode != 0) {
                if (d->cancelNext) {
                    fail(tr("Cancelled"));
                } else {
                    fail(tr("Couldn't prepare ISO image"));
                }
                return;
            }

            d->nextItem++;
            performNextAction();
            process->deleteLater();
        });

        QStringList mkisofsargs = {"-o", d->workDir.filePath(QStringLiteral("cd.iso")), "-V", d->albumTitle.left(32), "-A", QApplication::applicationName(), "-r", "-J", d->workDir.filePath(QStringLiteral("cd"))};
        tDebug("cdrdao") << "Calling mkisofs with arguments" << mkisofsargs;
        process->start("mkisofs", mkisofsargs);
    } else if (d->nextItem == d->sourceFiles.count() + 1) {
        d->description = tr("Preparing to burn");
        emit descriptionChanged(d->description);

        // After this point, cancelling may damage the CD
        d->warnCancel = true;

        // Generate the TOC file and call cdrdao
        QFile tocFile(d->workDir.filePath("contents.toc"));
        tocFile.open(QFile::WriteOnly);

        QTextStream tocStream(&tocFile);
        tocStream << "CD_ROM\n";
        tocStream << "TRACK MODE1\n";
        tocStream << "DATAFILE \"cd.iso\"\n";
        tocFile.close();

        // Call cdrdao to write to the disc
        QProcess* process = new QProcess();
        process->setProcessChannelMode(QProcess::MergedChannels);
        process->setWorkingDirectory(d->workDir.path());
        connect(process, &QProcess::readyRead, this, [this, process] {
            QByteArray peek = process->peek(1024);
            while (process->canReadLine() || peek.contains('\r')) {
                QString line;
                if (process->canReadLine()) {
                    line = process->readLine();
                } else {
                    line = process->read(peek.indexOf('\r') + 1);
                }
                line = line.trimmed();

                tDebug("cdrdao") << line;
                if (line.startsWith("Writing track")) {
                    QStringList parts = line.split(" ");

                    d->description = tr("Burning Disc");
                    emit descriptionChanged(d->description);

                    d->burnState = BurnJobMp3Private::Tracks;
                } else if (line.startsWith("Wrote ")) {
                    if (line.contains("blocks.")) {
                        d->description = tr("Finalising CD");
                        emit descriptionChanged(d->description);

                        d->canCancel = false;
                        emit canCancelChanged(false);

                        d->burnState = BurnJobMp3Private::LeadOut;
                    } else {
                        QStringList parts = line.split(" ");
                        quint64 partProgress = parts.at(1).toInt();
                        quint64 partMaxProgress = parts.at(3).toInt();

                        switch (d->burnState) {
                            case BurnJobMp3Private::LeadIn: // Take up 1/4 of the part progress
                                d->progress = partProgress;
                                d->maxProgress = partMaxProgress * 4;
                                break;
                            case BurnJobMp3Private::Tracks: // Take up 1/2 of the part progress
                                d->progress = partProgress + partMaxProgress / 2;
                                d->maxProgress = partMaxProgress * 2;

                                d->description = tr("Burning %1\n%2 of %3").arg(QLocale().quoteString(d->albumTitle), QLocale().formattedDataSize(parts.at(1).toULongLong() * 1048576), QLocale().formattedDataSize(parts.at(3).toULongLong() * 1048576));
                                emit descriptionChanged(d->description);
                                break;
                            case BurnJobMp3Private::LeadOut: // Take up 1/4 of the part progress
                                d->progress = partProgress + partMaxProgress * 3;
                                d->maxProgress = partMaxProgress * 4;
                                break;
                        }

                        // Make this stage the second half of the total progress
                        d->progress += d->maxProgress;
                        d->maxProgress *= 2;

                        emit totalProgressChanged(d->maxProgress);
                        emit progressChanged(d->progress);
                    }
                }

                peek = process->peek(1024);
            }
        });
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this, process](int exitCode, QProcess::ExitStatus status) {
            if (exitCode != 0) {
                if (d->cancelNext) {
                    fail(tr("Cancelled"));
                } else {
                    fail(tr("Couldn't burn tracks"));
                }
                return;
            }

            d->nextItem++;
            performNextAction();
            process->deleteLater();
        });

        QStringList daoArgs = {"write", "-n", "--device", d->diskObject->interface<BlockInterface>()->blockName(), "--driver", "generic-mmc-raw", "contents.toc"};
        tDebug("cdrdao") << "Calling cdrdao with arguments" << daoArgs;
        process->start("cdrdao", daoArgs);

        d->daoProcess = process;

        d->canCancel = true;
        emit canCancelChanged(true);
    } else {
        co_await d->diskObject->interface<BlockInterface>()->triggerReload();
        d->state = Finished;
        emit stateChanged(Finished);

        d->progress = 100;
        d->maxProgress = 100;
        emit totalProgressChanged(d->maxProgress);
        emit progressChanged(d->progress);

        d->description = tr("Burn Successful");
        emit descriptionChanged(d->description);

        d->workDir.remove();

        tInfo("cdrdao") << "Burn job completed successfully";

        // Fire a notification
        tNotification* notification = new tNotification(tr("Burn Successful"), tr("Burned %1 to disc").arg(QLocale().quoteString(d->albumTitle)));
        notification->post();
    }
}

void BurnJobMp3::fail(QString description) {
    d->state = Failed;
    emit stateChanged(Failed);

    d->description = description;
    emit descriptionChanged(d->description);

    d->workDir.remove();

    // Fire a notification
    tNotification* notification = new tNotification(tr("Burn Failure"), tr("Failed to burn %1 to disc").arg(QLocale().quoteString(d->albumTitle)));
    notification->post();
}

quint64 BurnJobMp3::progress() {
    return d->progress;
}

quint64 BurnJobMp3::totalProgress() {
    return d->maxProgress;
}

BurnJobMp3::State BurnJobMp3::state() {
    return d->state;
}
