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
#include "stationsearchwidget.h"
#include "ui_stationsearchwidget.h"

#include "radioinfoclient.h"
#include "stationwidget.h"

#include <QException>

struct StationSearchWidgetPrivate {
        //    QList<RadioInfoClient::Station> topVotedStations;
        QList<StationWidget*> topVotedWidgets;

        QList<StationWidget*> searchWidgets;
        quint16 searchNonce = 0;
};

StationSearchWidget::StationSearchWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::StationSearchWidget) {
    ui->setupUi(this);

    d = new StationSearchWidgetPrivate();

    //    ui->titleLabel->setBackButtonShown(true);

    //    QList<RadioInfoClient::Station> stations = RadioInfoClient::topVoted();
    connect(RadioInfoClient::instance(), &RadioInfoClient::ready, this, [=]() -> QCoro::Task<> {
        try {
            auto stations = co_await RadioInfoClient::topVoted();
            for (const RadioInfoClient::Station& station : stations) {
                d->topVotedWidgets.append(new StationWidget(station, this));
            }

            layoutTopVoted();
        } catch (QException ex) {
        }
    });
}

StationSearchWidget::~StationSearchWidget() {
    delete ui;
    delete d;
}

void StationSearchWidget::on_titleLabel_backButtonClicked() {
    emit done();
}

void StationSearchWidget::resizeEvent(QResizeEvent* event) {
    layoutTopVoted();
}

void StationSearchWidget::layoutTopVoted() {
    for (StationWidget* widget : d->topVotedWidgets) {
        ui->topVotedLayout->removeWidget(widget);
    }

    int cols = this->width() < SC_DPI(400) ? 1 : 2;

    for (int i = 0; i < d->topVotedWidgets.count(); i++) {
        StationWidget* widget = d->topVotedWidgets.value(i);
        ui->topVotedLayout->addWidget(widget, i / cols, i % cols);
        widget->show();
    }
}

QCoro::Task<> StationSearchWidget::on_searchBox_textChanged(const QString& arg1) {
    if (arg1.isEmpty()) {
        ui->stackedWidget->setCurrentWidget(ui->discoverWidget);
    } else {
        ui->stackedWidget->setCurrentWidget(ui->searchWidget);

        for (StationWidget* widget : d->searchWidgets) {
            ui->searchLayout->removeWidget(widget);
            widget->deleteLater();
        }
        d->searchWidgets.clear();

        quint16 nonce = ++d->searchNonce;
        auto stations = co_await RadioInfoClient::search(arg1);

        if (d->searchNonce != nonce) co_return;

        for (const RadioInfoClient::Station& station : stations) {
            StationWidget* widget = new StationWidget(station, this);
            ui->searchLayout->addWidget(widget);
            d->searchWidgets.append(widget);
        }
    }
}
