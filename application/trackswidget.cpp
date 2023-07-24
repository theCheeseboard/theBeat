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

#include "library/librarymanager.h"
#include "libraryerrorpopover.h"
#include <QUrl>
#include <playlist.h>
#include <statemanager.h>
#include <tpopover.h>

struct TracksWidgetPrivate {
        T_INJECTED(IUrlManager);
        LibraryModel* model = nullptr;
};

TracksWidget::TracksWidget(QWidget* parent, T_INJECTED(IUrlManager)) :
    AbstractLibraryBrowser(parent),
    ui(new Ui::TracksWidget) {
    ui->setupUi(this);

    d = new TracksWidgetPrivate();
    T_INJECT_SAVE_D(IUrlManager);
    connect(LibraryManager::instance(), &LibraryManager::libraryChanged, this, &TracksWidget::updateModel);
    connect(LibraryManager::instance(), &LibraryManager::libraryChanged, this, &TracksWidget::updateProcessing);
    connect(LibraryManager::instance(), &LibraryManager::isProcessingChanged, this, &TracksWidget::updateProcessing);

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Fade);
    ui->processingSpinner->setFixedSize(QSize(ui->processingTitle->height(), ui->processingTitle->height()));
    updateModel();
    updateProcessing();
}

TracksWidget::~TracksWidget() {
    delete d;
    delete ui;
}

AbstractLibraryBrowser::ListInformation TracksWidget::currentListInformation() {
    ListInformation info;
    if (ui->searchBox->text().isEmpty()) {
        info.name = tr("Tracks in Library");
    } else {
        info.name = tr("Search for %1").arg(QLocale().quoteString(ui->searchBox->text()));
    }

    for (int i = 0; i < d->model->rowCount(); i++) {
        QModelIndex index = d->model->index(i, 0);
        TrackInformation trackInfo;
        trackInfo.title = index.data(LibraryModel::TitleRole).toString();
        trackInfo.artist = index.data(LibraryModel::ArtistRole).toString();
        trackInfo.album = index.data(LibraryModel::AlbumRole).toString();
        trackInfo.trackNumber = index.data(LibraryModel::TrackRole).toInt();
        trackInfo.duration = index.data(LibraryModel::DurationRole).toULongLong();
        info.tracks.append(trackInfo);
    }

    return info;
}

void TracksWidget::setTopPadding(int padding) {
    ui->topWidget->setContentsMargins(0, padding, 0, 0);
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

void TracksWidget::updateProcessing() {
    if (LibraryManager::instance()->isProcessing() && LibraryManager::instance()->countTracks() == 0) {
        ui->stackedWidget->setCurrentWidget(ui->processingPage);
    } else {
        QTimer::singleShot(500, this, [this] {
            ui->stackedWidget->setCurrentWidget(ui->libraryPage);
        });
    }
}

void TracksWidget::on_enqueueAllButton_clicked() {
    for (int i = 0; i < d->model->rowCount(); i++) {
        if (d->model->index(i, 0).data(LibraryModel::ErrorRole).value<LibraryModel::Errors>() != LibraryModel::NoError) continue;

        MediaItem* item = T_INJECTED_SERVICE(IUrlManager)->itemForUrl(QUrl::fromLocalFile(d->model->index(i, 0).data(LibraryModel::PathRole).toString()));
        StateManager::instance()->playlist()->addItem(item);
    }
}
