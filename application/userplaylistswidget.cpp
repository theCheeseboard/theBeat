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

#include "common.h"
#include "library/librarymanager.h"
#include <QFileDialog>
#include <QMenu>
#include <QUrl>
#include <burnbackend.h>
#include <burnmanager.h>
#include <playlist.h>
#include <statemanager.h>
#include <tinputdialog.h>

struct UserPlaylistsWidgetPrivate {
        T_INJECTED(IUrlManager);
        int currentPlaylist = -1;
        QString currentPlaylistName;
};

UserPlaylistsWidget::UserPlaylistsWidget(QWidget* parent, T_INJECTED(IUrlManager)) :
    AbstractLibraryBrowser(parent),
    ui(new Ui::UserPlaylistsWidget) {
    ui->setupUi(this);

    d = new UserPlaylistsWidgetPrivate();
    T_INJECT_SAVE_D(IUrlManager);

    connect(LibraryManager::instance(), &LibraryManager::playlistsChanged, this, &UserPlaylistsWidget::updatePlaylists);
    connect(LibraryManager::instance(), &LibraryManager::playlistChanged, this, [this](int id) {
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

AbstractLibraryBrowser::ListInformation UserPlaylistsWidget::currentListInformation() {
    if (ui->stackedWidget->currentWidget() == ui->mainPage) return ListInformation();

    ListInformation info;
    info.name = d->currentPlaylistName;

    for (int i = 0; i < ui->tracksList->model()->rowCount(); i++) {
        QModelIndex index = ui->tracksList->model()->index(i, 0);
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

void UserPlaylistsWidget::on_createButton_clicked() {
    bool ok;
    QString playlistName = tInputDialog::getText(this->window(), tr("New Playlist"), tr("What name do you want to give to this playlist?"), QLineEdit::Normal, "", &ok);
    if (ok) {
        LibraryManager::instance()->createPlaylist(playlistName);
    }
}

void UserPlaylistsWidget::updatePlaylists() {
    ui->playlistsList->clear();

    for (int i = 0; i < LibraryManager::LastSmartPlaylist; i++) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(LibraryManager::instance()->smartPlaylistName(static_cast<LibraryManager::SmartPlaylist>(i)));
        item->setData(Qt::UserRole, i);
        item->setData(Qt::UserRole + 1, true);
        ui->playlistsList->addItem(item);
    }

    for (QPair<int, QString> playlist : LibraryManager::instance()->playlists()) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(playlist.second);
        item->setData(Qt::UserRole, playlist.first);
        item->setData(Qt::UserRole + 1, false);
        ui->playlistsList->addItem(item);
    }
}

void UserPlaylistsWidget::loadPlaylist(int id) {
    LibraryModel* model = LibraryManager::instance()->tracksByPlaylist(id);
    ui->tracksList->setModel(model);
    ui->stackedWidget->setCurrentWidget(ui->tracksPage);

    ui->tracksList->setCurrentPlaylistId(id);
    ui->playlistMenuButton->setMenu(playlistManagementMenu({id}));
    ui->playlistMenuButton->setVisible(true);

    d->currentPlaylist = id;
}

void UserPlaylistsWidget::loadSmartPlaylist(LibraryManager::SmartPlaylist smartPlaylist) {
    LibraryModel* model = LibraryManager::instance()->smartPlaylist(smartPlaylist);
    ui->tracksList->setModel(model);
    ui->stackedWidget->setCurrentWidget(ui->tracksPage);

    ui->tracksList->setCurrentPlaylistId(-1);
    ui->playlistMenuButton->setVisible(false);

    d->currentPlaylist = smartPlaylist;
}

void UserPlaylistsWidget::updateBurn() {
    ui->burnButton->setVisible(!StateManager::instance()->burn()->availableBackends().isEmpty());
}

QMenu* UserPlaylistsWidget::playlistManagementMenu(QList<int> playlists) {
    QMenu* menu = new QMenu();
    if (playlists.count() == 1) {
        QString playlistName;
        for (QPair<int, QString> playlist : LibraryManager::instance()->playlists()) {
            if (playlist.first == playlists.first()) {
                playlistName = playlist.second;
                break;
            }
        }
        menu->addSection(tr("For %1").arg(QLocale().quoteString(playlistName)));
        menu->addAction(QIcon::fromTheme("edit-rename"), tr("Rename"), this, [this, playlists, playlistName] {
            bool ok;
            QString name = tInputDialog::getText(this->window(), tr("Rename"), tr("What name do you want to give to this playlist?"), QLineEdit::Normal, playlistName, &ok);
            if (ok) {
                LibraryManager::instance()->renamePlaylist(playlists.first(), name);
                if (d->currentPlaylist == playlists.first()) ui->tracksTitle->setText(tr("Tracks in %1").arg(name));
            }
        });

        // TODO: Reimplement file export
        //        menu->addAction(QIcon::fromTheme("document-export"), tr("Export"), this, [=] {
        //            QFileDialog* dialog = new QFileDialog(this);
        //            dialog->setAcceptMode(QFileDialog::AcceptOpen);
        //            dialog->setNameFilters({"M3U8 Playlists (*.m3u8)"});
        //            dialog->setFileMode(QFileDialog::AnyFile);
        //            connect(dialog, &QFileDialog::fileSelected, this, [=](QString file) {
        //                QMediaPlaylist playlist;
        //                LibraryModel* model = LibraryManager::instance()->tracksByPlaylist(playlists.first());
        //
        //                for (int i = 0; i < model->rowCount(); i++) {
        //                    QModelIndex index = model->index(i, 0);
        //                    playlist.addMedia(QMediaContent(QUrl::fromLocalFile(index.data(LibraryModel::PathRole).toString())));
        //                }
        //
        //                model->deleteLater();
        //                playlist.save(QUrl::fromLocalFile(file), "m3u8");
        //            });
        //            connect(dialog, &QFileDialog::finished, dialog, &QFileDialog::deleteLater);
        //            dialog->open();
        //        });

        menu->addSeparator();

        QMenu* removeMenu = new QMenu(this);
        removeMenu->setIcon(QIcon::fromTheme("edit-delete"));
        removeMenu->setTitle(tr("Remove"));
        removeMenu->addSection(tr("Are you sure?"));
        removeMenu->addAction(QIcon::fromTheme("edit-delete"), tr("Remove"), this, [this, playlists] {
            LibraryManager::instance()->removePlaylist(playlists.first());
            if (d->currentPlaylist == playlists.first()) ui->stackedWidget->setCurrentWidget(ui->mainPage);
        });
        menu->addMenu(removeMenu);
    } else {
        menu->addSection(tr("For %n playlists", nullptr, ui->playlistsList->selectedItems().count()));

        QMenu* removeMenu = new QMenu(this);
        removeMenu->setIcon(QIcon::fromTheme("edit-delete"));
        removeMenu->setTitle(tr("Remove"));
        removeMenu->addSection(tr("Are you sure?"));
        removeMenu->addAction(QIcon::fromTheme("edit-delete"), tr("Remove"), this, [playlists] {
            for (int playlist : playlists) {
                LibraryManager::instance()->removePlaylist(playlist);
            }
        });
        menu->addMenu(removeMenu);
    }
    return menu;
}

void UserPlaylistsWidget::on_playlistsList_itemActivated(QListWidgetItem* item) {
    if (item->data(Qt::UserRole + 1) == true) {
        // Smart playlist
        LibraryManager::SmartPlaylist smartPlaylist = static_cast<LibraryManager::SmartPlaylist>(item->data(Qt::UserRole).toInt());
        ui->tracksTitle->setText(LibraryManager::instance()->smartPlaylistName(smartPlaylist));
        loadSmartPlaylist(smartPlaylist);
        d->currentPlaylistName = LibraryManager::instance()->smartPlaylistName(smartPlaylist);
    } else {
        ui->tracksTitle->setText(tr("Tracks in %1").arg(QLocale().quoteString(item->text())));
        loadPlaylist(item->data(Qt::UserRole).toInt());
        d->currentPlaylistName = item->text();
    }
}

void UserPlaylistsWidget::on_backButton_clicked() {
    ui->stackedWidget->setCurrentWidget(ui->mainPage);
}

void UserPlaylistsWidget::on_enqueueAllButton_clicked() {
    for (int i = 0; i < ui->tracksList->model()->rowCount(); i++) {
        if (ui->tracksList->model()->index(i, 0).data(LibraryModel::ErrorRole).value<LibraryModel::Errors>() != LibraryModel::NoError) continue;

        MediaItem* item = T_INJECTED_SERVICE(IUrlManager)->itemForUrl(QUrl::fromLocalFile(ui->tracksList->model()->index(i, 0).data(LibraryModel::PathRole).toString()));
        StateManager::instance()->playlist()->addItem(item);
    }
}

void UserPlaylistsWidget::on_burnButton_clicked() {
    QStringList files;
    for (int i = 0; i < ui->tracksList->model()->rowCount(); i++) {
        if (ui->tracksList->model()->index(i, 0).data(LibraryModel::ErrorRole).value<LibraryModel::Errors>() != LibraryModel::NoError) {
            // TODO: Do something!!!
            continue;
        }

        files.append(ui->tracksList->model()->index(i, 0).data(LibraryModel::PathRole).toString());
    }

    Common::showBurnMenu(files, d->currentPlaylistName, ui->burnButton);
}

void UserPlaylistsWidget::on_playlistsList_customContextMenuRequested(const QPoint& pos) {
    QList<int> selectedPlaylists;
    for (QListWidgetItem* item : ui->playlistsList->selectedItems()) {
        if (item->data(Qt::UserRole + 1).toBool()) return;
        selectedPlaylists.append(item->data(Qt::UserRole).toInt());
    }

    QMenu* menu = playlistManagementMenu(selectedPlaylists);
    connect(menu, &QMenu::aboutToHide, menu, &QMenu::deleteLater);
    menu->popup(ui->playlistsList->mapToGlobal(pos));
}

void UserPlaylistsWidget::on_playAllButton_clicked() {
    StateManager::instance()->playlist()->clear();
    ui->enqueueAllButton->click();
}

void UserPlaylistsWidget::on_shuffleButton_clicked() {
    ui->enqueueAllButton->click();
    StateManager::instance()->playlist()->setShuffle(true);
    StateManager::instance()->playlist()->next();
}
