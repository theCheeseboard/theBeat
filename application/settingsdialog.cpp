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
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <tsettings.h>
#include <tapplication.h>
#include <tcsdtools.h>
#include "library/librarymanager.h"

struct SettingsDialogPrivate {
    tSettings settings;
};

SettingsDialog::SettingsDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog) {
    ui->setupUi(this);
    d = new SettingsDialogPrivate();

    ui->titleLabel->setBackButtonShown(true);
    ui->leftWidget->setFixedWidth(SC_DPI(300));
    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Lift);

    ui->resetLibraryButton->setProperty("type", "destructive");

    ui->trackChangeNotification->setChecked(d->settings.value("notifications/trackChange").toBool());
    ui->listWidget->selectionModel()->setCurrentIndex(ui->listWidget->model()->index(0, 0), QItemSelectionModel::SelectCurrent);
    ui->useSsdsCheckbox->setChecked(d->settings.value("appearance/useSsds").toBool());

#ifdef Q_OS_WIN
    if (d->settings.value("theme/mode").toString() == "light") {
        ui->lightButton->setChecked(true);
    } else {
        ui->darkButton->setChecked(true);
    }
#else
    ui->coloursWidget->setVisible(false);
    ui->line_2->setVisible(false);
#endif
}

SettingsDialog::~SettingsDialog() {
    delete d;
    delete ui;
}

void SettingsDialog::on_titleLabel_backButtonClicked() {
    this->close();
}

void SettingsDialog::on_resetLibraryButton_clicked() {
    LibraryManager::instance()->erase();
    tApplication::restart();
}

void SettingsDialog::on_trackChangeNotification_toggled(bool checked) {
    d->settings.setValue("notifications/trackChange", checked);
}

void SettingsDialog::on_useSsdsCheckbox_toggled(bool checked) {
    d->settings.setValue("appearance/useSsds", checked);
}

void SettingsDialog::on_lightButton_toggled(bool checked)
{
    if (checked) {
        d->settings.setValue("theme/mode", "light");
    }
}

void SettingsDialog::on_darkButton_toggled(bool checked)
{
    if (checked) {
        d->settings.setValue("theme/mode", "dark");
    }
}
