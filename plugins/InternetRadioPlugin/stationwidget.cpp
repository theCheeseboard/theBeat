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
#include "stationwidget.h"
#include "ui_stationwidget.h"

#include "radioinfoclient.h"
#include <QException>
#include <QPointer>
#include <playlist.h>
#include <statemanager.h>

struct StationWidgetPrivate {
        T_INJECTED(IUrlManager);
        RadioInfoClient::Station station;
};

StationWidget::StationWidget(RadioInfoClient::Station station, QWidget* parent, T_INJECTED(IUrlManager)) :
    QWidget(parent),
    ui(new Ui::StationWidget) {
    ui->setupUi(this);

    d = new StationWidgetPrivate();
    d->station = station;
    T_INJECT_SAVE_D(IUrlManager);

    ui->nameLabel->setText(station.name);
    ui->secondaryLabel->setText(station.country);

    this->load();
    if (this->layoutDirection() == Qt::RightToLeft) {
        ui->nameLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->secondaryLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
}

StationWidget::~StationWidget() {
    delete ui;
    delete d;
}

void StationWidget::on_playButton_clicked() {
    // Count the click
    RadioInfoClient::countClick(d->station);

    // Add the station URL to the queue and play it
    MediaItem* item = T_INJECTED_SERVICE(IUrlManager)->itemForUrl(QUrl(d->station.streamUrl));
    StateManager::instance()->playlist()->addItem(item);
    StateManager::instance()->playlist()->setCurrentItem(item);
}

void StationWidget::resizeEvent(QResizeEvent* event) {
    ui->iconLabel->setFixedWidth(this->height());
}

QCoro::Task<> StationWidget::load() {
    QPointer<StationWidget> pointer = this;
    try {
        auto px = co_await RadioInfoClient::getIcon(d->station);
        if (!pointer) co_return;

        ui->iconLabel->setPixmap(px);
    } catch (QException ex) {
        if (!pointer) co_return;

        ui->iconLabel->setVisible(false);
    }
}
