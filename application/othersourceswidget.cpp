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
#include "othersourceswidget.h"
#include "ui_othersourceswidget.h"

#include <pluginmediasource.h>
#include <sourcemanager.h>
#include <statemanager.h>

struct OtherSourcesWidgetPrivate {
        QMap<QListWidgetItem*, PluginMediaSource*> listItems;
};

OtherSourcesWidget::OtherSourcesWidget(QWidget* parent) :
    AbstractLibraryBrowser(parent),
    ui(new Ui::OtherSourcesWidget) {
    ui->setupUi(this);

    d = new OtherSourcesWidgetPrivate();
    ui->sourcesList->setFixedWidth(SC_DPI(300));
    ui->sourcesList->setIconSize(SC_DPI_T(QSize(32, 32), QSize));

    connect(StateManager::instance()->sources(), &SourceManager::sourceAdded, this, [=](PluginMediaSource* source) {
        QListWidgetItem* item = new QListWidgetItem();
        ui->sourcesList->addItem(item);

        connect(source, &PluginMediaSource::nameChanged, this, [=](QString text) {
            item->setText(text);
        });
        connect(source, &PluginMediaSource::iconChanged, this, [=](QIcon icon) {
            item->setIcon(icon);
        });
        item->setText(source->name());
        item->setIcon(source->icon());
        d->listItems.insert(item, source);

        ui->stackedWidget->addWidget(source->widget());

        ui->mainStack->setCurrentWidget(ui->sourcesPage);
    });
    connect(StateManager::instance()->sources(), &SourceManager::sourceRemoved, this, [=](PluginMediaSource* source) {
        QListWidgetItem* item = ui->sourcesList->takeItem(ui->sourcesList->row(d->listItems.key(source)));
        d->listItems.remove(item);
        ui->stackedWidget->removeWidget(source->widget());
        source->disconnect(this);
        delete item;

        if (ui->stackedWidget->count() == 0) ui->mainStack->setCurrentWidget(ui->noSourcesPage);
    });

    ui->mainStack->setCurrentAnimation(tStackedWidget::Fade);
    ui->mainStack->setCurrentWidget(ui->noSourcesPage, false);
}

OtherSourcesWidget::~OtherSourcesWidget() {
    delete d;
    delete ui;
}

void OtherSourcesWidget::setTopPadding(int padding) {
    ui->sourcesLayout->setContentsMargins(0, padding, 0, 0);
}

AbstractLibraryBrowser::ListInformation OtherSourcesWidget::currentListInformation() {
    AbstractLibraryBrowser* currentBrowser = qobject_cast<AbstractLibraryBrowser*>(ui->stackedWidget->currentWidget());
    if (!currentBrowser || currentBrowser->currentListInformation().tracks.isEmpty()) {
        return ListInformation();
    } else {
        return currentBrowser->currentListInformation();
    }
}

void OtherSourcesWidget::on_sourcesList_currentRowChanged(int currentRow) {
    QListWidgetItem* item = ui->sourcesList->item(currentRow);
    if (item) ui->stackedWidget->setCurrentWidget(d->listItems.value(item)->widget());
}
