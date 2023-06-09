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
#ifndef PRINTSETTINGS_H
#define PRINTSETTINGS_H

#include <tprintpopover.h>

namespace Ui {
    class PrintSettings;
}

class PrintSettings : public tPrintPopoverCustomPrintSettingsWidget {
        Q_OBJECT

    public:
        explicit PrintSettings(QWidget* parent = nullptr);
        ~PrintSettings();

        bool printTrackListing();

    private slots:
        void on_trackListingButton_toggled(bool checked);

        void on_jewelCaseButton_toggled(bool checked);

    private:
        Ui::PrintSettings* ui;
};

#endif // PRINTSETTINGS_H
