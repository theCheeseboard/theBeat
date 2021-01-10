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
#include "burnjobwidget.h"
#include "ui_burnjobwidget.h"

#include "burnjob.h"
#include "burnjobmp3.h"

struct BurnJobWidgetPrivate {
    tJob* parentJob;
};

BurnJobWidget::BurnJobWidget(BurnJob* parent) :
    QWidget(nullptr),
    ui(new Ui::BurnJobWidget) {
    ui->setupUi(this);

    d = new BurnJobWidgetPrivate();
    d->parentJob = parent;

    connect(parent, &BurnJob::totalProgressChanged, this, [ = ](quint64 totalProgress) {
        ui->progressBar->setMaximum(totalProgress);
    });
    connect(parent, &BurnJob::progressChanged, this, [ = ](quint64 progress) {
        ui->progressBar->setValue(progress);
    });
    connect(parent, &BurnJob::descriptionChanged, this, [ = ](QString description) {
        ui->statusLabel->setText(description);
    });
    connect(parent, &BurnJob::canCancelChanged, this, [ = ](bool canCancel) {
        ui->cancelButton->setEnabled(canCancel);
    });
    ui->progressBar->setMaximum(parent->totalProgress());
    ui->progressBar->setValue(parent->progress());
    ui->statusLabel->setText(parent->description());
    ui->cancelButton->setEnabled(parent->canCancel());
}

BurnJobWidget::BurnJobWidget(BurnJobMp3* parent) :
    QWidget(nullptr),
    ui(new Ui::BurnJobWidget) {
    ui->setupUi(this);

    d = new BurnJobWidgetPrivate();
    d->parentJob = parent;

    connect(parent, &BurnJobMp3::totalProgressChanged, this, [ = ](quint64 totalProgress) {
        ui->progressBar->setMaximum(totalProgress);
    });
    connect(parent, &BurnJobMp3::progressChanged, this, [ = ](quint64 progress) {
        ui->progressBar->setValue(progress);
    });
    connect(parent, &BurnJobMp3::descriptionChanged, this, [ = ](QString description) {
        ui->statusLabel->setText(description);
    });
    connect(parent, &BurnJobMp3::canCancelChanged, this, [ = ](bool canCancel) {
        ui->cancelButton->setEnabled(canCancel);
    });
    ui->progressBar->setMaximum(parent->totalProgress());
    ui->progressBar->setValue(parent->progress());
    ui->statusLabel->setText(parent->description());
    ui->cancelButton->setEnabled(parent->canCancel());
}

BurnJobWidget::~BurnJobWidget() {
    delete d;
    delete ui;
}

void BurnJobWidget::on_cancelButton_clicked() {
    if (qobject_cast<BurnJob*>(d->parentJob)) {
        qobject_cast<BurnJob*>(d->parentJob)->cancel();
    } else if (qobject_cast<BurnJobMp3*>(d->parentJob)) {
        qobject_cast<BurnJobMp3*>(d->parentJob)->cancel();
    }
}
