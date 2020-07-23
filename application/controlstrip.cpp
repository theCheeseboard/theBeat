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
#include "controlstrip.h"
#include "ui_controlstrip.h"

#include <statemanager.h>
#include <playlist.h>
#include <the-libs_global.h>

struct ControlStripPrivate {
    MediaItem* currentItem = nullptr;
};

ControlStrip::ControlStrip(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ControlStrip) {
    ui->setupUi(this);

    d = new ControlStripPrivate();

    ui->playPauseButton->setIconSize(SC_DPI_T(QSize(32, 32), QSize));
    connect(StateManager::instance()->playlist(), &Playlist::stateChanged, this, &ControlStrip::updateState);
    connect(StateManager::instance()->playlist(), &Playlist::currentItemChanged, this, &ControlStrip::updateCurrentItem);

    updateCurrentItem();
}

ControlStrip::~ControlStrip() {
    delete d;
    delete ui;
}

void ControlStrip::on_playPauseButton_clicked() {
    switch (StateManager::instance()->playlist()->state()) {
        case Playlist::Playing:
            StateManager::instance()->playlist()->pause();
            break;
        case Playlist::Paused:
        case Playlist::Stopped:
            StateManager::instance()->playlist()->play();
            break;
    }
}

void ControlStrip::updateState() {
    switch (StateManager::instance()->playlist()->state()) {
        case Playlist::Playing:
            ui->playPauseButton->setIcon(QIcon::fromTheme("media-playback-pause"));
            break;
        case Playlist::Paused:
        case Playlist::Stopped:
            ui->playPauseButton->setIcon(QIcon::fromTheme("media-playback-start"));
            break;
    }
}

void ControlStrip::updateCurrentItem() {
    if (d->currentItem) {
        d->currentItem->disconnect(this);
    }
    d->currentItem = StateManager::instance()->playlist()->currentItem();
    if (d->currentItem) {
        connect(d->currentItem, &MediaItem::metadataChanged, this, &ControlStrip::updateMetadata);
        updateMetadata();
    }
}

void ControlStrip::updateMetadata() {
    ui->titleLabel->setText(d->currentItem->title());
}

void ControlStrip::on_skipBackButton_clicked() {
    StateManager::instance()->playlist()->previous();
}

void ControlStrip::on_skipNextButton_clicked() {
    StateManager::instance()->playlist()->next();
}
