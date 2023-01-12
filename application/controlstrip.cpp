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

#include "common.h"
#include "currenttrackpopover.h"
#include <QGraphicsOpacityEffect>
#include <QMainWindow>
#include <QMenu>
#include <QPointer>
#include <controlstripmanager.h>
#include <playlist.h>
#include <statemanager.h>
#include <tpopover.h>
#include <tsettings.h>
#include <tvariantanimation.h>

struct ControlStripPrivate {
        MediaItem* currentItem = nullptr;
        bool isCollapsed = true;

        QPalette standardPal;
        QPointer<tPopover> zenModePopover;
        QWidget* zenBacking;

        tSettings settings;

        tVariantAnimation* volumeBarAnim;
        bool inZenMode = false;
};

ControlStrip::ControlStrip(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ControlStrip) {
    ui->setupUi(this);

    d = new ControlStripPrivate();

    ui->playPauseButton->setIconSize(SC_DPI_T(QSize(32, 32), QSize));
    connect(StateManager::instance()->playlist(), &Playlist::stateChanged, this, &ControlStrip::updateState);
    connect(StateManager::instance()->playlist(), &Playlist::currentItemChanged, this, &ControlStrip::updateCurrentItem);
    connect(StateManager::instance()->playlist(), &Playlist::repeatOneChanged, ui->repeatOneButton, &QToolButton::setChecked);
    connect(StateManager::instance()->playlist(), &Playlist::shuffleChanged, ui->shuffleButton, &QToolButton::setChecked);
    connect(StateManager::instance()->playlist(), &Playlist::volumeChanged, this, [=] {
        ui->volumeSlider->setValue(StateManager::instance()->playlist()->volume() * 100);
    });
    ui->volumeSlider->setValue(StateManager::instance()->playlist()->volume() * 100);

    if (StateManager::instance()->playlist()->items().isEmpty()) this->setFixedHeight(0);
    ui->volumeWidget->installEventFilter(this);
    ui->volumeSlider->setFixedWidth(0);

    d->standardPal = this->palette();

    d->volumeBarAnim = new tVariantAnimation(this);
    d->volumeBarAnim->setDuration(500);
    d->volumeBarAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(d->volumeBarAnim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
        this->setFixedHeight(value.toInt());
    });
    connect(d->volumeBarAnim, &tVariantAnimation::finished, this, [=] {
        if (d->volumeBarAnim->endValue().toInt() != 0) this->setFixedHeight(QWIDGETSIZE_MAX);
    });

    StateManager::instance()->playlist()->setRepeatAll(d->settings.value("playback/repeatAll").toBool());

    QMenu* repeatAllMenu = new QMenu();
    repeatAllMenu->addSection(tr("Repeat Options"));
    QAction* playQueueAction = repeatAllMenu->addAction(tr("Repeat Play Queue"), [=] {
        d->settings.setValue("playback/repeatAll", !StateManager::instance()->playlist()->repeatAll());
    });
    playQueueAction->setCheckable(true);
    connect(StateManager::instance()->playlist(), &Playlist::repeatAllChanged, this, [=](bool repeatAll) {
        d->settings.setValue("playback/repeatAll", repeatAll);
    });
    connect(&d->settings, &tSettings::settingChanged, this, [=](QString key, QVariant value) {
        if (key == "playback/repeatAll" && StateManager::instance()->playlist()->repeatAll() != value.toBool()) {
            playQueueAction->setChecked(value.toBool());
            StateManager::instance()->playlist()->setRepeatAll(value.toBool());
        }
    });
    playQueueAction->setChecked(StateManager::instance()->playlist()->repeatAll());
    ui->repeatOneButton->setMenu(repeatAllMenu);

    QMenu* pauseMenu = new QMenu();
    pauseMenu->addSection(tr("Playback Options"));
    QAction* pauseAfterCurrentTrackAction = pauseMenu->addAction(QIcon::fromTheme("media-playback-pause"), tr("Pause after current track"));
    connect(pauseAfterCurrentTrackAction, &QAction::toggled, this, [=](bool checked) {
        StateManager::instance()->playlist()->setPauseAfterCurrentTrack(checked);
    });
    pauseAfterCurrentTrackAction->setCheckable(true);
    pauseAfterCurrentTrackAction->setChecked(StateManager::instance()->playlist()->pauseAfterCurrentTrack());
    connect(StateManager::instance()->playlist(), &Playlist::pauseAfterCurrentTrackChanged, this, [=](bool pauseAfterCurrentTrack) {
        pauseAfterCurrentTrackAction->setChecked(pauseAfterCurrentTrack);
    });
    ui->playPauseButton->setMenu(pauseMenu);

    connect(StateManager::instance()->controlStrip(), &ControlStripManager::buttonAdded, this, [=](QWidget* button) {
        ui->customButtonsLayout->addWidget(button);
    });

    // Ensure that the seeker and transport controls are always LTR
    ui->transportControlsWidget->setLayoutDirection(Qt::LeftToRight);
    ui->seekerWidget->setLayoutDirection(Qt::LeftToRight);

    updateCurrentItem();
}

ControlStrip::~ControlStrip() {
    delete d;
    delete ui;
}

void ControlStrip::enterZenMode() {
    if (d->inZenMode) return;

    CurrentTrackPopover* track = new CurrentTrackPopover(this);

#ifdef Q_OS_MAC
    QMainWindow* parentWindow = static_cast<QMainWindow*>(this->window());

    d->zenBacking = new QWidget();

    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(d->zenBacking);
    effect->setOpacity(0);
    d->zenBacking->setGraphicsEffect(effect);

    tVariantAnimation* opacityAnim = new tVariantAnimation(d->zenBacking);
    opacityAnim->setStartValue(0.0);
    opacityAnim->setEndValue(1.0);
    opacityAnim->setDuration(500);
    opacityAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(opacityAnim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
        effect->setOpacity(value.toReal());
    });
    connect(opacityAnim, &tVariantAnimation::finished, this, [=] {
        opacityAnim->deleteLater();
        effect->deleteLater();
    });
    opacityAnim->start();

    QBoxLayout* backingLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    backingLayout->setContentsMargins(0, 0, 0, 0);
    backingLayout->addWidget(track);
    d->zenBacking->setLayout(backingLayout);

    track->setParent(d->zenBacking);

    d->zenBacking->setParent(parentWindow->centralWidget());
    d->zenBacking->resize(parentWindow->centralWidget()->size());
    d->zenBacking->move(0, 0);
    d->zenBacking->setAutoFillBackground(true);
    d->zenBacking->show();

    track->setBacking(d->zenBacking);
#else
    d->zenModePopover = new tPopover(track);
    d->zenModePopover->setPopoverSide(tPopover::Bottom);
    d->zenModePopover->setPopoverWidth(SC_DPI(-100));
    connect(d->zenModePopover, &tPopover::dismissed, d->zenModePopover, &tPopover::deleteLater);
    connect(d->zenModePopover, &tPopover::dismissed, track, &CurrentTrackPopover::deleteLater);
    connect(d->zenModePopover, &tPopover::dismissed, this, [this] {
        d->inZenMode = false;
        emit inZenModeChanged(false);
    });
    d->zenModePopover->show(this->window());
#endif

    d->inZenMode = true;
    emit inZenModeChanged(true);
}

void ControlStrip::leaveZenMode() {
    if (!d->inZenMode) return;

#ifdef Q_OS_MAC
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(d->zenBacking);
    effect->setOpacity(1);
    d->zenBacking->setGraphicsEffect(effect);

    tVariantAnimation* opacityAnim = new tVariantAnimation(d->zenBacking);
    opacityAnim->setStartValue(1.0);
    opacityAnim->setEndValue(0.0);
    opacityAnim->setDuration(500);
    opacityAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(opacityAnim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
        effect->setOpacity(value.toReal());
    });
    connect(opacityAnim, &tVariantAnimation::finished, this, [=] {
        d->zenBacking->deleteLater();
    });
    opacityAnim->start();
#else
    d->zenModePopover->dismiss();
#endif

    d->inZenMode = false;
    emit inZenModeChanged(false);
}

bool ControlStrip::inZenMode() {
    return d->inZenMode;
}

void ControlStrip::on_playPauseButton_clicked() {
    StateManager::instance()->playlist()->playPause();
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
        connect(d->currentItem, &MediaItem::elapsedChanged, this, &ControlStrip::updateBar);
        connect(d->currentItem, &MediaItem::durationChanged, this, &ControlStrip::updateBar);
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
        this->setPalette(d->standardPal);
    } else {
        ui->artLabel->setPixmap(QPixmap::fromImage(image).scaled(SC_DPI_T(QSize(48, 48), QSize), Qt::KeepAspectRatio, Qt::SmoothTransformation));

        qulonglong red = 0, green = 0, blue = 0;

        QPalette pal = this->palette();
        int totalPixels = 0;
        for (int i = 0; i < image.width(); i++) {
            for (int j = 0; j < image.height(); j++) {
                QColor c = image.pixelColor(i, j);
                if (c.alpha() != 0) {
                    red += c.red();
                    green += c.green();
                    blue += c.blue();
                    totalPixels++;
                }
            }
        }

        QColor c = QColor(red / totalPixels, green / totalPixels, blue / totalPixels).darker(200);

        pal.setColor(QPalette::Window, c);
        pal.setColor(QPalette::WindowText, Qt::white);
        this->setPalette(pal);
    }
}

void ControlStrip::updateBar() {
    QSignalBlocker blocker(ui->progressSlider);
    ui->progressSlider->setMaximum(d->currentItem->duration());
    ui->progressSlider->setValue(d->currentItem->elapsed());

    ui->durationLabel->setText(Common::durationToString(d->currentItem->duration(), true));
    ui->elapsedLabel->setText(Common::durationToString(d->currentItem->elapsed()));
}

void ControlStrip::expand() {
    if (!d->isCollapsed) return;

    d->volumeBarAnim->stop();
    d->volumeBarAnim->setStartValue(this->height());
    d->volumeBarAnim->setEndValue(this->sizeHint().height());
    d->volumeBarAnim->start();
    d->isCollapsed = false;
}

void ControlStrip::collapse() {
    if (d->isCollapsed) return;

    d->volumeBarAnim->stop();
    d->volumeBarAnim->setStartValue(this->height());
    d->volumeBarAnim->setEndValue(0);
    d->volumeBarAnim->start();
    d->isCollapsed = true;
}

void ControlStrip::on_skipBackButton_clicked() {
    StateManager::instance()->playlist()->previous();
}

void ControlStrip::on_skipNextButton_clicked() {
    StateManager::instance()->playlist()->next();
}

void ControlStrip::on_progressSlider_valueChanged(int value) {
    d->currentItem->seek(value);
}

void ControlStrip::on_repeatOneButton_toggled(bool checked) {
    StateManager::instance()->playlist()->setRepeatOne(checked);
}

void ControlStrip::on_shuffleButton_toggled(bool checked) {
    StateManager::instance()->playlist()->setShuffle(checked);
}

void ControlStrip::on_volumeSlider_valueChanged(int value) {
    StateManager::instance()->playlist()->setVolume(static_cast<double>(value) / 100);
}

bool ControlStrip::eventFilter(QObject* watched, QEvent* event) {
    if (watched == ui->volumeWidget) {
        if (event->type() == QEvent::Enter) {
            tVariantAnimation* anim = new tVariantAnimation(this);
            anim->setStartValue(ui->volumeSlider->width());
            anim->setEndValue(SC_DPI(150));
            anim->setDuration(500);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            connect(anim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
                ui->volumeSlider->setFixedWidth(value.toInt());
                ui->volumeWidget->updateGeometry();
            });
            connect(anim, &tVariantAnimation::finished, anim, &tVariantAnimation::deleteLater);
            anim->start();
        } else if (event->type() == QEvent::Leave) {
            tVariantAnimation* anim = new tVariantAnimation(this);
            anim->setStartValue(ui->volumeSlider->width());
            anim->setEndValue(0);
            anim->setDuration(500);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            connect(anim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
                ui->volumeSlider->setFixedWidth(value.toInt());
                ui->volumeWidget->updateGeometry();
            });
            connect(anim, &tVariantAnimation::finished, anim, &tVariantAnimation::deleteLater);
            anim->start();
        }
    }
    return false;
}

void ControlStrip::on_upButton_clicked() {
    enterZenMode();
}

void ControlStrip::on_repeatOneButton_customContextMenuRequested(const QPoint& pos) {
    Q_UNUSED(pos);
    ui->repeatOneButton->showMenu();
}

void ControlStrip::on_playPauseButton_customContextMenuRequested(const QPoint& pos) {
    Q_UNUSED(pos);
    ui->playPauseButton->showMenu();
}
