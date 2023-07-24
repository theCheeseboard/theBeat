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
#ifndef ARTISTSALBUMSWIDGET_H
#define ARTISTSALBUMSWIDGET_H

#include <abstractlibrarybrowser.h>
#include <dependencyinjection/tinjectedpointer.h>
#include <iurlmanager.h>

namespace Ui {
    class ArtistsAlbumsWidget;
}

class QListWidgetItem;
struct ArtistsAlbumsWidgetPrivate;
class ArtistsAlbumsWidget : public AbstractLibraryBrowser {
        Q_OBJECT

    public:
        explicit ArtistsAlbumsWidget(QWidget* parent = nullptr, T_INJECT(IUrlManager));
        ~ArtistsAlbumsWidget();

        enum Type {
            Artists,
            Albums
        };

        void setType(Type type);

        void setTopPadding(int padding);
        ListInformation currentListInformation();

        void changeItem(QString text);

    private slots:
        void on_initialList_itemActivated(QListWidgetItem* item);

        void on_backButton_clicked();

        void on_enqueueAllButton_clicked();

        void on_shuffleButton_clicked();

        void on_playAllButton_clicked();

        void on_burnButton_clicked();

    private:
        Ui::ArtistsAlbumsWidget* ui;
        ArtistsAlbumsWidgetPrivate* d;

        void updateData();
        void updateBurn();

        bool eventFilter(QObject* watched, QEvent* event);
};

#endif // ARTISTSALBUMSWIDGET_H
