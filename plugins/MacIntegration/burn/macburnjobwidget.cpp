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
#include "macburnjobwidget.h"
#include "ui_macburnjobwidget.h"

#include "macburnjob.h"

struct MacBurnJobWidgetPrivate {
    MacBurnJob* parentJob;
};

MacBurnJobWidget::MacBurnJobWidget(MacBurnJob* parent) :
    QWidget(nullptr),
    ui(new Ui::MacBurnJobWidget) {
    ui->setupUi(this);

    d = new MacBurnJobWidgetPrivate();
    d->parentJob = parent;

    connect(parent, &MacBurnJob::totalProgressChanged, this, [ = ](quint64 totalProgress) {
        ui->progressBar->setMaximum(totalProgress);
    });
    connect(parent, &MacBurnJob::progressChanged, this, [ = ](quint64 progress) {
        ui->progressBar->setValue(progress);
    });
    connect(parent, &MacBurnJob::descriptionChanged, this, [ = ](QString description) {
        ui->statusLabel->setText(description);
    });
    connect(parent, &MacBurnJob::canCancelChanged, this, [ = ](bool canCancel) {
        ui->cancelButton->setEnabled(canCancel);
    });
    connect(parent, &MacBurnJob::stateChanged, this, &MacBurnJobWidget::updateState);
    ui->progressBar->setMaximum(parent->totalProgress());
    ui->progressBar->setValue(parent->progress());
    ui->statusLabel->setText(parent->description());
    ui->cancelButton->setEnabled(parent->canCancel());
    this->updateState(parent->state());
}

MacBurnJobWidget::~MacBurnJobWidget() {
    delete d;
    delete ui;
}

void MacBurnJobWidget::on_cancelButton_clicked() {
    d->parentJob->cancel();
}

void MacBurnJobWidget::updateState(tJob::State state) {
    if (state == tJob::Processing || state == tJob::RequiresAttention) {
        ui->progressWidget->setVisible(true);
    } else {
        ui->progressWidget->setVisible(false);
    }
}
