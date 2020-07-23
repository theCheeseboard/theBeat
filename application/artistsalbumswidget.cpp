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
#include "artistsalbumswidget.h"
#include "ui_artistsalbumswidget.h"

#include "qtmultimedia/qtmultimediamediaitem.h"
#include <statemanager.h>
#include <playlist.h>
#include <QUrl>
#include "library/librarymanager.h"

struct ArtistsAlbumsWidgetPrivate {
    ArtistsAlbumsWidget::Type type;
};

ArtistsAlbumsWidget::ArtistsAlbumsWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ArtistsAlbumsWidget) {
    ui->setupUi(this);

    d = new ArtistsAlbumsWidgetPrivate();
    connect(LibraryManager::instance(), &LibraryManager::libraryChanged, this, &ArtistsAlbumsWidget::updateData);

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Lift);
}

ArtistsAlbumsWidget::~ArtistsAlbumsWidget() {
    delete ui;
}

void ArtistsAlbumsWidget::setType(ArtistsAlbumsWidget::Type type) {
    d->type = type;
    ui->titleLabel->setText(type == Albums ? tr("Albums in Library") : tr("Artists in Library"));
    updateData();
}

void ArtistsAlbumsWidget::updateData() {
    QStringList data = d->type == Albums ? LibraryManager::instance()->albums() : LibraryManager::instance()->artists();
    ui->initialList->clear();
    for (QString item : data) {
        ui->initialList->addItem(item);
    }
}

void ArtistsAlbumsWidget::on_initialList_itemActivated(QListWidgetItem* item) {
    ui->tracksTitle->setText(d->type == Albums ? tr("Tracks in %1").arg(item->text()) : tr("Tracks by %1").arg(item->text()));
    LibraryModel* model = d->type == Albums ? LibraryManager::instance()->tracksByAlbum(item->text()) : LibraryManager::instance()->tracksByArtist(item->text());
    ui->tracksList->setModel(model);
    ui->stackedWidget->setCurrentWidget(ui->tracksPage);
}

void ArtistsAlbumsWidget::on_backButton_clicked() {
    ui->stackedWidget->setCurrentWidget(ui->mainPage);
}

void ArtistsAlbumsWidget::on_tracksList_activated(const QModelIndex& index) {
    QtMultimediaMediaItem* item = new QtMultimediaMediaItem(QUrl::fromLocalFile(index.data(LibraryModel::PathRole).toString()));
    StateManager::instance()->playlist()->addItem(item);
    StateManager::instance()->playlist()->setCurrentItem(item);
}
