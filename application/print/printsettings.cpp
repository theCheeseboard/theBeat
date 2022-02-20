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
#include "printsettings.h"
#include "ui_printsettings.h"

PrintSettings::PrintSettings(QWidget* parent) :
    tPrintPopoverCustomPrintSettingsWidget(parent),
    ui(new Ui::PrintSettings) {
    ui->setupUi(this);
}

PrintSettings::~PrintSettings() {
    delete ui;
}

bool PrintSettings::printTrackListing() {
    return ui->trackListingButton->isChecked();
}

void PrintSettings::on_trackListingButton_toggled(bool checked) {
    if (checked) emit requestPaintUpdate();
}

void PrintSettings::on_jewelCaseButton_toggled(bool checked) {
    if (checked) emit requestPaintUpdate();
}
