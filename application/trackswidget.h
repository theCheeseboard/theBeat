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
#ifndef TRACKSWIDGET_H
#define TRACKSWIDGET_H

#include <abstractlibrarybrowser.h>
#include <dependencyinjection/tinjectedpointer.h>
#include <iurlmanager.h>

namespace Ui {
    class TracksWidget;
}

struct TracksWidgetPrivate;
class TracksWidget : public AbstractLibraryBrowser {
        Q_OBJECT

    public:
        explicit TracksWidget(QWidget* parent = nullptr, T_INJECT(IUrlManager));
        ~TracksWidget();

        ListInformation currentListInformation();
        void setTopPadding(int padding);

    private slots:
        void on_searchBox_textEdited(const QString& arg1);

        void on_enqueueAllButton_clicked();

    private:
        Ui::TracksWidget* ui;
        TracksWidgetPrivate* d;

        void updateModel();
        void updateProcessing();
};

#endif // TRACKSWIDGET_H
