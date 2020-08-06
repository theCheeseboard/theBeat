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
#include "importcdpopover.h"
#include "ui_importcdpopover.h"

#include <QFileDialog>
#include <QStandardPaths>
#include "importcdjob.h"
#include <tjobmanager.h>

struct ImportCdPopoverPrivate {
    QString blockDevice;
    QList<TrackInfoPtr> trackInfo;
};

ImportCdPopover::ImportCdPopover(QString blockDevice, QString albumName, QList<TrackInfoPtr> trackInfo, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ImportCdPopover) {
    ui->setupUi(this);

    d = new ImportCdPopoverPrivate();
    d->blockDevice = blockDevice;
    d->trackInfo = trackInfo;

    ui->titleLabel->setText(tr("Import %1").arg(albumName));
    ui->titleLabel->setBackButtonShown(true);
    ui->importOptionsWidget->setFixedWidth(SC_DPI(600));

    ui->importFolderLocation->setText(QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + "/theBeat/" + albumName);
}

ImportCdPopover::~ImportCdPopover() {
    delete d;
    delete ui;
}

void ImportCdPopover::on_titleLabel_backButtonClicked() {
    emit done();
}

void ImportCdPopover::on_browseImportFolderButton_clicked() {
    QFileDialog* d = new QFileDialog(this);
    d->setAcceptMode(QFileDialog::AcceptOpen);
    d->setFileMode(QFileDialog::DirectoryOnly);
    connect(d, &QFileDialog::fileSelected, this, [ = ](QString file) {
        ui->importFolderLocation->setText(file);
    });
    connect(d, &QFileDialog::finished, d, &QFileDialog::deleteLater);
    d->open();
}

void ImportCdPopover::on_importButton_clicked() {
    ImportCdJob* job = new ImportCdJob(d->blockDevice, d->trackInfo, ui->importFolderLocation->text(), 1);
    tJobManager::trackJob(job);
    emit done();
}
