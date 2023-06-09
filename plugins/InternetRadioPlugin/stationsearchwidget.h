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
#ifndef STATIONSEARCHWIDGET_H
#define STATIONSEARCHWIDGET_H

#include <QCoroTask>
#include <QWidget>

namespace Ui {
    class StationSearchWidget;
}

struct StationSearchWidgetPrivate;
class StationSearchWidget : public QWidget {
        Q_OBJECT

    public:
        explicit StationSearchWidget(QWidget* parent = nullptr);
        ~StationSearchWidget();

    signals:
        void done();

    private slots:
        void on_titleLabel_backButtonClicked();

        QCoro::Task<> on_searchBox_textChanged(const QString& arg1);

    private:
        Ui::StationSearchWidget* ui;
        StationSearchWidgetPrivate* d;

        void resizeEvent(QResizeEvent* event);

        void layoutTopVoted();
};

#endif // STATIONSEARCHWIDGET_H
