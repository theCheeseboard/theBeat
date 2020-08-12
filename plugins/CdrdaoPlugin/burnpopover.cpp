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

#include <QDBusInterface>
#include <QProcess>
#include <taglib/fileref.h>
#include <taglib/audioproperties.h>
#include <tjobmanager.h>
#include "burnjob.h"

struct BurnPopoverPrivate {
    QStringList files;
    QString blockDevice;
    quint64 playlistLength = 0;
};

BurnPopover::BurnPopover(QStringList files, QString blockDevice, QString albumName, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::BurnPopover) {
    ui->setupUi(this);

    d = new BurnPopoverPrivate();
    d->files = files;
    d->blockDevice = blockDevice;

    ui->titleLabel->setText(tr("Burn %1").arg(albumName));
    ui->titleLabel->setBackButtonShown(true);
    ui->burnOptionsWidget->setFixedWidth(SC_DPI(600));

    ui->albumNameEdit->setText(albumName);

    QPalette pal = ui->warningFrame->palette();
    pal.setColor(QPalette::Window, QColor(255, 100, 0));
    pal.setColor(QPalette::WindowText, Qt::white);
    ui->warningFrame->setPalette(pal);

    for (QString file : files) {
        TagLib::FileRef tagFile(file.toStdString().data());
        if (tagFile.audioProperties()) {
            d->playlistLength += tagFile.audioProperties()->lengthInMilliseconds();
        }
    }

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
    BurnJob* job = new BurnJob(d->files, d->blockDevice, ui->albumNameEdit->text());
    tJobManager::trackJob(job);
    emit done();
}

void BurnPopover::on_albumNameEdit_textChanged(const QString& arg1) {
    ui->titleLabel->setText(tr("Burn %1").arg(arg1));
}

void BurnPopover::updateCd() {
    QDBusInterface blockInterface("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2/block_devices/" + d->blockDevice, "org.freedesktop.UDisks2.Block", QDBusConnection::systemBus());
    QDBusObjectPath cdDrivePath = blockInterface.property("Drive").value<QDBusObjectPath>();
    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", cdDrivePath.path(), "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(updateCd()));

    QDBusInterface cdDriveInterface("org.freedesktop.UDisks2", cdDrivePath.path(), "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus());
    QString media = cdDriveInterface.property("Media").toString();
    bool mediaBlank = cdDriveInterface.property("OpticalBlank").toBool();
    if (!QStringList({"optical_cd_r", "optical_cd_rw"}).contains(media)) {
        ui->warningText->setText(tr("Insert a CD-R or a CD-RW into the drive."));
        ui->warningFrame->setVisible(true);
        ui->burnButton->setEnabled(false);
    } else if (!mediaBlank && media == "optical_cd_rw") {
        ui->warningText->setText(tr("The CD in the drive is not blank. By burning to this CD, you will erase all the data currently on it."));
        ui->warningFrame->setVisible(true);
        ui->burnButton->setEnabled(true);
    } else if (!mediaBlank && media == "optical_cd_r") {
        ui->warningText->setText(tr("The CD-R in the drive has already been written."));
        ui->warningFrame->setVisible(true);
        ui->burnButton->setEnabled(false);
    } else {
        //Call cdrdao to make sure this CD is large enough to fit the data
        QProcess cdrdao;
        cdrdao.start("cdrdao", {"disk-info"});
        cdrdao.waitForFinished();

        quint64 capacity = 0;
        while (cdrdao.canReadLine()) {
            QString line = cdrdao.readLine().trimmed();
            if (line.startsWith("Total Capacity")) {
                QStringList parts = line.split(" ", Qt::SkipEmptyParts);
                if (parts.count() < 4) continue;
                QString duration = parts.at(3);

                QStringList durationParts = duration.split(":");
                if (durationParts.count() < 3) continue;
                capacity = durationParts.at(0).toInt() * 60 * 1000 + durationParts.at(1).toInt() * 1000 + durationParts.at(2).toInt() * 100;
            }
        }

        if (capacity < d->playlistLength) {
            ui->warningText->setText(tr("This playlist is too long to fit on the CD."));
            ui->warningFrame->setVisible(true);
            ui->burnButton->setEnabled(false);
        } else if (media == "optical_cd_rw") {
            ui->warningText->setText(tr("The CD in the drive is rewritable, so the burned CD may not work on older CD players."));
            ui->warningFrame->setVisible(true);
            ui->burnButton->setEnabled(true);
        } else {
            ui->warningFrame->setVisible(false);
            ui->burnButton->setEnabled(true);
        }
    }
}

