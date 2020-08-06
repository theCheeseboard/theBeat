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
#ifndef IMPORTCDPOPOVER_H
#define IMPORTCDPOPOVER_H

#include <QWidget>
#include "trackinfo.h"

namespace Ui {
    class ImportCdPopover;
}

struct ImportCdPopoverPrivate;
class ImportCdPopover : public QWidget {
        Q_OBJECT

    public:
        explicit ImportCdPopover(QString blockDevice, QString albumName, QList<TrackInfoPtr> trackInfo, QWidget* parent = nullptr);
        ~ImportCdPopover();

    private slots:
        void on_titleLabel_backButtonClicked();

        void on_browseImportFolderButton_clicked();

        void on_importButton_clicked();

    signals:
        void done();

    private:
        Ui::ImportCdPopover* ui;
        ImportCdPopoverPrivate* d;
};

#endif // IMPORTCDPOPOVER_H
