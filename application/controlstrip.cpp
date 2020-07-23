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
#include <tvariantanimation.h>

struct ControlStripPrivate {
    MediaItem* currentItem = nullptr;
    bool isCollapsed = true;
};

ControlStrip::ControlStrip(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ControlStrip) {
    ui->setupUi(this);

    d = new ControlStripPrivate();

    ui->playPauseButton->setIconSize(SC_DPI_T(QSize(32, 32), QSize));
    connect(StateManager::instance()->playlist(), &Playlist::stateChanged, this, &ControlStrip::updateState);
    connect(StateManager::instance()->playlist(), &Playlist::currentItemChanged, this, &ControlStrip::updateCurrentItem);

    this->setFixedHeight(0);

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
            expand();
            ui->playPauseButton->setIcon(QIcon::fromTheme("media-playback-pause"));
            break;
        case Playlist::Paused:
            expand();
            ui->playPauseButton->setIcon(QIcon::fromTheme("media-playback-start"));
            break;
        case Playlist::Stopped:
            collapse();
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

    QStringList parts;
    parts.append(QLocale().createSeparatedList(d->currentItem->authors()));
    parts.append(d->currentItem->album());

    parts.removeAll("");
    ui->metadataLabel->setText(parts.join(" · "));

    QImage image = d->currentItem->albumArt();
    if (image.isNull()) {
        ui->artLabel->setPixmap(QIcon::fromTheme("audio").pixmap(SC_DPI_T(QSize(48, 48), QSize)));
    } else {
        ui->artLabel->setPixmap(QPixmap::fromImage(image).scaled(SC_DPI_T(QSize(48, 48), QSize), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void ControlStrip::expand() {
    if (!d->isCollapsed) return;

    tVariantAnimation* anim = new tVariantAnimation(this);
    anim->setStartValue(this->height());
    anim->setEndValue(this->sizeHint().height());
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tVariantAnimation::valueChanged, this, [ = ](QVariant value) {
        this->setFixedHeight(value.toInt());
    });
    connect(anim, &tVariantAnimation::finished, this, [ = ] {
        this->setFixedHeight(QWIDGETSIZE_MAX);
        anim->deleteLater();
    });
    anim->start();
    d->isCollapsed = false;
}

void ControlStrip::collapse() {
    if (d->isCollapsed) return;

    tVariantAnimation* anim = new tVariantAnimation(this);
    anim->setStartValue(this->height());
    anim->setEndValue(this->sizeHint().height());
    anim->setDuration(0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tVariantAnimation::valueChanged, this, [ = ](QVariant value) {
        this->setFixedHeight(value.toInt());
    });
    connect(anim, &tVariantAnimation::finished, anim, &tVariantAnimation::deleteLater);
    anim->start();
    d->isCollapsed = true;
}

void ControlStrip::on_skipBackButton_clicked() {
    StateManager::instance()->playlist()->previous();
}

void ControlStrip::on_skipNextButton_clicked() {
    StateManager::instance()->playlist()->next();
}
