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
#include "userplaylistswidget.h"
#include "ui_userplaylistswidget.h"

#include <QMenu>
#include <QInputDialog>
#include <QUrl>
#include <statemanager.h>
#include <playlist.h>
#include <burnmanager.h>
#include <burnbackend.h>
#include "qtmultimedia/qtmultimediamediaitem.h"
#include "library/librarymanager.h"
#include "common.h"

struct UserPlaylistsWidgetPrivate {
    int currentPlaylist = -1;
    QString currentPlaylistName;
};

UserPlaylistsWidget::UserPlaylistsWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::UserPlaylistsWidget) {
    ui->setupUi(this);

    d = new UserPlaylistsWidgetPrivate();

    connect(LibraryManager::instance(), &LibraryManager::playlistsChanged, this, &UserPlaylistsWidget::updatePlaylists);
    connect(LibraryManager::instance(), &LibraryManager::playlistChanged, this, [ = ](int id) {
        if (id == d->currentPlaylist) loadPlaylist(id);
    });
    updatePlaylists();

    connect(StateManager::instance()->burn(), &BurnManager::backendRegistered, this, &UserPlaylistsWidget::updateBurn);
    connect(StateManager::instance()->burn(), &BurnManager::backendDeregistered, this, &UserPlaylistsWidget::updateBurn);
    updateBurn();

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Lift);
}

UserPlaylistsWidget::~UserPlaylistsWidget() {
    delete d;
    delete ui;
}

void UserPlaylistsWidget::setTopPadding(int padding) {
    ui->playlistsTopWidget->setContentsMargins(0, padding, 0, 0);
    ui->tracksTopWidget->setContentsMargins(0, padding, 0, 0);
}

void UserPlaylistsWidget::on_createButton_clicked() {
    bool ok;
    QString playlistName = QInputDialog::getText(this, tr("Playlist Name"), tr("Playlist Name"), QLineEdit::Normal, "", &ok);
    if (ok) {
        LibraryManager::instance()->createPlaylist(playlistName);
    }
}

void UserPlaylistsWidget::updatePlaylists() {
    ui->playlistsList->clear();
    for (QPair<int, QString> playlist : LibraryManager::instance()->playlists()) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(playlist.second);
        item->setData(Qt::UserRole, playlist.first);
        ui->playlistsList->addItem(item);
    }
}

void UserPlaylistsWidget::loadPlaylist(int id) {
    LibraryModel* model = LibraryManager::instance()->tracksByPlaylist(id);
    ui->tracksList->setModel(model);
    ui->stackedWidget->setCurrentWidget(ui->tracksPage);

    d->currentPlaylist = id;
}

void UserPlaylistsWidget::updateBurn() {
    ui->burnButton->setVisible(!StateManager::instance()->burn()->availableBackends().isEmpty());
}

void UserPlaylistsWidget::on_playlistsList_itemActivated(QListWidgetItem* item) {
    ui->tracksTitle->setText(tr("Tracks in %1").arg(item->text()));
    loadPlaylist(item->data(Qt::UserRole).toInt());
    d->currentPlaylistName = item->text();
}

void UserPlaylistsWidget::on_backButton_clicked() {
    ui->stackedWidget->setCurrentWidget(ui->mainPage);
}

void UserPlaylistsWidget::on_enqueueAllButton_clicked() {
    for (int i = 0; i < ui->tracksList->model()->rowCount(); i++) {
        if (ui->tracksList->model()->index(i, 0).data(LibraryModel::ErrorRole).value<LibraryModel::Errors>() != LibraryModel::NoError) continue;

        QtMultimediaMediaItem* item = new QtMultimediaMediaItem(QUrl::fromLocalFile(ui->tracksList->model()->index(i, 0).data(LibraryModel::PathRole).toString()));
        StateManager::instance()->playlist()->addItem(item);
    }
}

void UserPlaylistsWidget::on_burnButton_clicked() {
    QStringList files;
    for (int i = 0; i < ui->tracksList->model()->rowCount(); i++) {
        if (ui->tracksList->model()->index(i, 0).data(LibraryModel::ErrorRole).value<LibraryModel::Errors>() != LibraryModel::NoError) {
            //TODO: Do something!!!
            continue;
        }

        files.append(ui->tracksList->model()->index(i, 0).data(LibraryModel::PathRole).toString());
    }

    Common::showBurnMenu(files, d->currentPlaylistName, ui->burnButton);
}
