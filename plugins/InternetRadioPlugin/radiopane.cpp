/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2021 Victor Tran
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
#include "radiopane.h"
#include "ui_radiopane.h"

#include <statemanager.h>
#include <sourcemanager.h>
#include <pluginmediasource.h>

struct RadioPanePrivate {
    PluginMediaSource* source;
};

RadioPane::RadioPane(QWidget* parent) :
    AbstractLibraryBrowser(parent),
    ui(new Ui::RadioPane) {
    ui->setupUi(this);

    d = new RadioPanePrivate();
    d->source = new PluginMediaSource(this);
    d->source->setName(tr("Internet Radio"));
    d->source->setIcon(QIcon::fromTheme("radio"));

    StateManager::instance()->sources()->addSource(d->source);

    this->layout()->setContentsMargins(0, StateManager::instance()->sources()->padTop(), 0, 0);

    connect(ui->favouriteStations, &FavouriteStationsWidget::addStation, this, [ = ] {
        ui->stackedWidget->setCurrentWidget(ui->searchPage);
    });
    connect(ui->searchPage, &StationSearchWidget::done, this, [ = ] {
        ui->stackedWidget->setCurrentWidget(ui->favouriteStations);
    });

    //TODO: Remove once favourite stations is implemented
    ui->stackedWidget->setCurrentWidget(ui->searchPage);
}

RadioPane::~RadioPane() {
    StateManager::instance()->sources()->removeSource(d->source);

    delete ui;
    delete d;
}

AbstractLibraryBrowser::ListInformation RadioPane::currentListInformation()
{
    return ListInformation();
}
