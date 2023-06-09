/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2022 Victor Tran
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
#ifndef PRINTPOPOVER_H
#define PRINTPOPOVER_H

#include <QWidget>
#include <abstractlibrarybrowser.h>

namespace Ui {
    class PrintPopover;
}

class QPrinter;
struct PrintPopoverPrivate;
class PrintPopover : public QWidget {
        Q_OBJECT

    public:
        explicit PrintPopover(AbstractLibraryBrowser::ListInformation listInformation, QWidget* parent = nullptr);
        ~PrintPopover();

    signals:
        void done();

    private slots:
        void on_titleLabel_backButtonClicked();

        void on_trackListingButton_toggled(bool checked);

        void on_jewelCaseButton_toggled(bool checked);

        void on_printerBox_currentIndexChanged(int index);

        void on_printButton_clicked();

        void on_pageSizeBox_currentIndexChanged(int index);

        void on_grayscaleBox_toggled(bool checked);

        void on_duplexBox_toggled(bool checked);

    private:
        Ui::PrintPopover* ui;
        PrintPopoverPrivate* d;

        void updatePageSizes();

        void paintPrinter(QPrinter* printer);
        void paintTrackListing(QPrinter* printer);
};

#endif // PRINTPOPOVER_H
