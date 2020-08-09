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

#include <tjobmanager.h>
#include "burnjob.h"

struct BurnPopoverPrivate {
    QStringList files;
    QString blockDevice;
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
