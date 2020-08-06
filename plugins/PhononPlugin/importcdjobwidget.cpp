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
#include "importcdjobwidget.h"
#include "ui_importcdjobwidget.h"

struct ImportCdJobWidgetPrivate {
    ImportCdJob* parent;
};

ImportCdJobWidget::ImportCdJobWidget(ImportCdJob* parent) :
    QWidget(nullptr),
    ui(new Ui::ImportCdJobWidget) {
    ui->setupUi(this);

    d = new ImportCdJobWidgetPrivate();
    d->parent = parent;

    connect(parent, &ImportCdJob::totalProgressChanged, this, [ = ](quint64 totalProgress) {
        ui->progressBar->setValue(totalProgress);
    });
    connect(parent, &ImportCdJob::progressChanged, this, [ = ](quint64 progress) {
        ui->progressBar->setValue(progress);
    });
    connect(parent, &ImportCdJob::descriptionChanged, this, [ = ](QString description) {
        ui->statusLabel->setText(description);
    });
    connect(parent, &ImportCdJob::canCancelChanged, this, [ = ](bool canCancel) {
        ui->cancelButton->setEnabled(canCancel);
    });
    ui->progressBar->setMaximum(parent->totalProgress());
    ui->progressBar->setValue(parent->progress());
    ui->statusLabel->setText(parent->description());
    ui->cancelButton->setEnabled(parent->canCancel());
}

ImportCdJobWidget::~ImportCdJobWidget() {
    delete d;
    delete ui;
}

void ImportCdJobWidget::on_cancelButton_clicked() {
    d->parent->cancel();
}
