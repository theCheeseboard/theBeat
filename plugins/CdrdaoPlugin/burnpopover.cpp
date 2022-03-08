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
#include "burnpopover.h"
#include "ui_burnpopover.h"

#include "burnjob.h"
#include "burnjobmp3.h"
#include "fullburnjob.h"
#include <QDBusInterface>
#include <QProcess>
#include <taglib/audioproperties.h>
#include <taglib/fileref.h>
#include <tjobmanager.h>

#include <DriveObjects/blockinterface.h>
#include <DriveObjects/diskobject.h>
#include <DriveObjects/driveinterface.h>

struct BurnPopoverPrivate {
        QStringList files;
        QPointer<DiskObject> diskObject;
        quint64 playlistLength = 0;
};

BurnPopover::BurnPopover(QStringList files, DiskObject* diskObject, QString albumName, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::BurnPopover) {
    ui->setupUi(this);

    d = new BurnPopoverPrivate();
    d->files = files;
    d->diskObject = diskObject;

    ui->titleLabel->setBackButtonShown(true);
    ui->titleLabel_2->setBackButtonShown(true);
    ui->burnOptionsWidget->setFixedWidth(SC_DPI(600));
    ui->burnConfirmWidget->setFixedWidth(SC_DPI(600));

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::SlideHorizontal);
    ui->doBurnButton->setProperty("type", "destructive");

    ui->albumNameEdit->setText(albumName);

    QPalette pal = ui->warningFrame->palette();
    pal.setColor(QPalette::Window, QColor(255, 100, 0));
    pal.setColor(QPalette::WindowText, Qt::white);
    ui->warningFrame->setPalette(pal);

    for (QString file : files) {
        TagLib::FileRef tagFile(file.toStdString().data());
        if (tagFile.audioProperties()) {
            d->playlistLength += tagFile.audioProperties()->length() * 1000;
        }
    }

    connect(d->diskObject, &DiskObject::destroyed, this, &BurnPopover::done);
    connect(d->diskObject->interface<BlockInterface>()->drive(), &DriveInterface::changed, this, &BurnPopover::updateCd);
    updateCd();
}

BurnPopover::~BurnPopover() {
    delete d;
    delete ui;
}

void BurnPopover::on_titleLabel_backButtonClicked() {
    emit done();
}

void BurnPopover::on_burnButton_clicked() {
    if (d->diskObject->interface<BlockInterface>()->drive()->opticalBlank()) {
        doBurn();
    } else {
        ui->stackedWidget->setCurrentWidget(ui->eraseWarningPage);
    }
}

void BurnPopover::on_albumNameEdit_textChanged(const QString& arg1) {
    ui->titleLabel->setText(tr("Burn %1").arg(QLocale().quoteString(arg1)));
    ui->titleLabel_2->setText(tr("Burn %1").arg(QLocale().quoteString(arg1)));
}

void BurnPopover::updateCd() {
    if (!d->diskObject) return;
    DriveInterface* drive = d->diskObject->interface<BlockInterface>()->drive();

    if (!QList<DriveInterface::MediaFormat>({DriveInterface::CdR, DriveInterface::CdRw}).contains(drive->media())) {
        ui->warningText->setText(tr("Insert a CD-R or a CD-RW into the drive."));
        ui->warningFrame->setVisible(true);
        ui->burnButton->setEnabled(false);
    } else if (!drive->opticalBlank() && drive->media() == DriveInterface::CdR) {
        ui->warningText->setText(tr("The CD-R in the drive has already been written."));
        ui->warningFrame->setVisible(true);
        ui->burnButton->setEnabled(false);
    } else {
        quint64 capacity = 0;
        if (ui->audioCdButton->isChecked()) {
            // Call cdrdao to make sure this CD is large enough to fit the data
            QProcess cdrdao;
            cdrdao.start("cdrdao", {"--device", d->diskObject->interface<BlockInterface>()->blockName(), "disk-info"});
            cdrdao.waitForFinished();

            while (cdrdao.canReadLine()) {
                QString line = cdrdao.readLine().trimmed();
                if (line.startsWith("Total Capacity")) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                    QStringList parts = line.split(" ", Qt::SkipEmptyParts);
#else
                    QStringList parts = line.split(" ", QString::SkipEmptyParts);
#endif
                    if (parts.count() < 4) continue;
                    QString duration = parts.at(3);

                    QStringList durationParts = duration.split(":");
                    if (durationParts.count() < 3) continue;
                    capacity = durationParts.at(0).toInt() * 60 * 1000 + durationParts.at(1).toInt() * 1000 + durationParts.at(2).toInt() * 100;
                }
            }
        }

        if (capacity < d->playlistLength && capacity != 0) {
            ui->warningText->setText(tr("This playlist is too long to fit on the CD."));
            ui->warningFrame->setVisible(true);
            ui->burnButton->setEnabled(false);
        } else if (drive->media() == DriveInterface::CdRw) {
            ui->warningText->setText(tr("The CD in the drive is rewritable, so the burned CD may not work on older CD players."));
            ui->warningFrame->setVisible(true);
            ui->burnButton->setEnabled(true);
        } else {
            ui->warningFrame->setVisible(false);
            ui->burnButton->setEnabled(true);
        }
    }
}

void BurnPopover::on_audioCdButton_toggled(bool checked) {
    if (checked) updateCd();
}

void BurnPopover::on_mp3CdButton_toggled(bool checked) {
    if (checked) updateCd();
}

void BurnPopover::on_titleLabel_2_backButtonClicked() {
    ui->stackedWidget->setCurrentWidget(ui->burnPage);
}

void BurnPopover::on_doBurnButton_clicked() {
    doBurn();
}

void BurnPopover::doBurn() {
    if (!d->diskObject) return;

    AbstractBurnJob* mainBurnJob;
    if (ui->mp3CdButton->isChecked()) {
        mainBurnJob = new BurnJobMp3(d->files, d->diskObject, ui->albumNameEdit->text());
    } else {
        mainBurnJob = new BurnJob(d->files, d->diskObject, ui->albumNameEdit->text());
    }

    FullBurnJob* fullBurnJob = new FullBurnJob(d->diskObject, mainBurnJob);
    tJobManager::trackJob(fullBurnJob);

    fullBurnJob->start();

    emit done();
}
