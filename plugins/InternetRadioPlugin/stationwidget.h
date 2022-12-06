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
#ifndef STATIONWIDGET_H
#define STATIONWIDGET_H

#include "radioinfoclient.h"
#include <QCoroTask>
#include <QWidget>

namespace Ui {
    class StationWidget;
}

struct StationWidgetPrivate;
class StationWidget : public QWidget {
        Q_OBJECT

    public:
        explicit StationWidget(RadioInfoClient::Station station, QWidget* parent = nullptr);
        ~StationWidget();

    private slots:
        void on_playButton_clicked();

    private:
        Ui::StationWidget* ui;
        StationWidgetPrivate* d;

        void resizeEvent(QResizeEvent* event);
        QCoro::Task<> load();
};

#endif // STATIONWIDGET_H
