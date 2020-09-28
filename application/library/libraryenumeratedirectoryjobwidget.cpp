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
#include "libraryenumeratedirectoryjobwidget.h"
#include "ui_libraryenumeratedirectoryjobwidget.h"

#include <tjob.h>

LibraryEnumerateDirectoryJobWidget::LibraryEnumerateDirectoryJobWidget(tJob* parentJob, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::LibraryEnumerateDirectoryJobWidget) {
    ui->setupUi(this);

    connect(parentJob, &tJob::totalProgressChanged, ui->progressBar, &QProgressBar::setMaximum);
    connect(parentJob, &tJob::progressChanged, ui->progressBar, &QProgressBar::setValue);

    ui->progressBar->setMaximum(parentJob->totalProgress());
    ui->progressBar->setValue(parentJob->progress());
}

LibraryEnumerateDirectoryJobWidget::~LibraryEnumerateDirectoryJobWidget() {
    delete ui;
}
