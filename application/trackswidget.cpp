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
#include "trackswidget.h"
#include "ui_trackswidget.h"

#include <QUrl>
#include <statemanager.h>
#include <playlist.h>
#include "qtmultimedia/qtmultimediamediaitem.h"
#include "library/librarymanager.h"

struct TracksWidgetPrivate {
    LibraryModel* model = nullptr;
};

TracksWidget::TracksWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::TracksWidget) {
    ui->setupUi(this);

    d = new TracksWidgetPrivate();
    connect(LibraryManager::instance(), &LibraryManager::libraryChanged, this, &TracksWidget::updateModel);

    updateModel();
}

TracksWidget::~TracksWidget() {
    delete d;
    delete ui;
}

void TracksWidget::on_listView_activated(const QModelIndex& index) {
    QtMultimediaMediaItem* item = new QtMultimediaMediaItem(QUrl::fromLocalFile(index.data(LibraryModel::PathRole).toString()));
    StateManager::instance()->playlist()->addItem(item);
    StateManager::instance()->playlist()->setCurrentItem(item);
}

void TracksWidget::on_searchBox_textEdited(const QString& arg1) {
    Q_UNUSED(arg1)
    updateModel();
}

void TracksWidget::updateModel() {
    if (d->model) d->model->deleteLater();
    QString query = ui->searchBox->text();
    if (query.isEmpty()) {
        d->model = LibraryManager::instance()->allTracks();
    } else {
        d->model = LibraryManager::instance()->searchTracks(query);
    }
    ui->listView->setModel(d->model);
}
