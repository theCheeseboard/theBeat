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
#ifndef FAVOURITESTATIONSWIDGET_H
#define FAVOURITESTATIONSWIDGET_H

#include <QWidget>

namespace Ui {
    class FavouriteStationsWidget;
}

struct FavouriteStationsWidgetPrivate;
class FavouriteStationsWidget : public QWidget {
        Q_OBJECT

    public:
        explicit FavouriteStationsWidget(QWidget* parent = nullptr);
        ~FavouriteStationsWidget();

    signals:
        void addStation();

    private slots:
        void on_searchForStationButton_clicked();

    private:
        Ui::FavouriteStationsWidget* ui;
        FavouriteStationsWidgetPrivate* d;
};

#endif // FAVOURITESTATIONSWIDGET_H
