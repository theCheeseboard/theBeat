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
        AbstractBurnJob* parentJob;
};

BurnJobWidget::BurnJobWidget(AbstractBurnJob* parent) :
    QWidget(nullptr),
    ui(new Ui::BurnJobWidget) {
    ui->setupUi(this);

    d = new BurnJobWidgetPrivate();
    d->parentJob = parent;

    connect(parent, &AbstractBurnJob::totalProgressChanged, this, [=](quint64 totalProgress) {
        ui->progressBar->setMaximum(totalProgress);
    });
    connect(parent, &AbstractBurnJob::progressChanged, this, [=](quint64 progress) {
        ui->progressBar->setValue(progress);
    });
    connect(parent, &AbstractBurnJob::descriptionChanged, this, [=](QString description) {
        ui->statusLabel->setText(description);
    });
    connect(parent, &AbstractBurnJob::canCancelChanged, this, [=](bool canCancel) {
        ui->cancelButton->setEnabled(canCancel);
    });
    connect(parent, &AbstractBurnJob::stateChanged, this, &BurnJobWidget::updateState);
    ui->progressBar->setMaximum(parent->totalProgress());
    ui->progressBar->setValue(parent->progress());
    ui->statusLabel->setText(parent->description());
    ui->cancelButton->setEnabled(parent->canCancel());
    this->updateState(parent->state());
}

BurnJobWidget::~BurnJobWidget() {
    delete d;
    delete ui;
}

void BurnJobWidget::on_cancelButton_clicked() {
    d->parentJob->cancel();
}

void BurnJobWidget::updateState(tJob::State state) {
    if (state == tJob::Processing || state == tJob::RequiresAttention) {
        ui->progressWidget->setVisible(true);
    } else {
        ui->progressWidget->setVisible(false);
    }
}
