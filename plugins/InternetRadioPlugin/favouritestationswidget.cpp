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
#include "favouritestationswidget.h"
#include "ui_favouritestationswidget.h"

#include "radioinfoclient.h"

struct FavouriteStationsWidgetPrivate {

};

FavouriteStationsWidget::FavouriteStationsWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::FavouriteStationsWidget) {
    ui->setupUi(this);

    d = new FavouriteStationsWidgetPrivate();

    ui->noStationsIcon->setPixmap(QIcon::fromTheme("radio").pixmap(SC_DPI_T(QSize(128, 128), QSize)));
}

FavouriteStationsWidget::~FavouriteStationsWidget() {
    delete ui;
    delete d;
}

void FavouriteStationsWidget::on_searchForStationButton_clicked() {
    emit addStation();
}
