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
#include "artistsalbumswidget.h"
#include "ui_artistsalbumswidget.h"

#include <statemanager.h>
#include <playlist.h>
#include <QUrl>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <burnbackend.h>
#include <burnmanager.h>
#include <QMenu>
#include <tpaintcalculator.h>
#include <urlmanager.h>
#include "common.h"
#include "library/librarymanager.h"

struct ArtistsAlbumsWidgetPrivate {
    ArtistsAlbumsWidget::Type type;
    int topPadding = 0;

    QImage playlistBackground;
    QString listName;
};

ArtistsAlbumsWidget::ArtistsAlbumsWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ArtistsAlbumsWidget) {
    ui->setupUi(this);

    d = new ArtistsAlbumsWidgetPrivate();
    connect(LibraryManager::instance(), &LibraryManager::libraryChanged, this, &ArtistsAlbumsWidget::updateData);

    connect(StateManager::instance()->burn(), &BurnManager::backendRegistered, this, &ArtistsAlbumsWidget::updateBurn);
    connect(StateManager::instance()->burn(), &BurnManager::backendDeregistered, this, &ArtistsAlbumsWidget::updateBurn);
    updateBurn();

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Lift);

    ui->topWidget->installEventFilter(this);
}

ArtistsAlbumsWidget::~ArtistsAlbumsWidget() {
    delete ui;
}

void ArtistsAlbumsWidget::setType(ArtistsAlbumsWidget::Type type) {
    d->type = type;
    ui->titleLabel->setText(type == Albums ? tr("Albums in Library") : tr("Artists in Library"));
    updateData();
}

void ArtistsAlbumsWidget::setTopPadding(int padding) {
    ui->mainTopWidget->setContentsMargins(0, padding, 0, 0);
    ui->topWidget->setContentsMargins(0, padding, 0, 0);

    d->topPadding = padding;
}

void ArtistsAlbumsWidget::updateData() {
    QStringList data = d->type == Albums ? LibraryManager::instance()->albums() : LibraryManager::instance()->artists();
    ui->initialList->clear();
    for (QString item : data) {
        ui->initialList->addItem(item);
    }
}

void ArtistsAlbumsWidget::updateBurn() {
    ui->burnButton->setVisible(!StateManager::instance()->burn()->availableBackends().isEmpty());
}

bool ArtistsAlbumsWidget::eventFilter(QObject* watched, QEvent* event) {
    if (watched == ui->topWidget && event->type() == QEvent::Paint) {
        QPainter* painter = new QPainter(ui->topWidget);

        tPaintCalculator paintCalculator;
        paintCalculator.setPainter(painter);
        paintCalculator.setDrawBounds(ui->topWidget->size());

        QColor backgroundCol = this->palette().color(QPalette::Window);
        if ((backgroundCol.red() + backgroundCol.green() + backgroundCol.blue()) / 3 < 127) {
            backgroundCol = QColor(0, 0, 0, 150);
        } else {
            backgroundCol = QColor(255, 255, 255, 150);
        }

        if (d->playlistBackground.isNull()) {
//            QSvgRenderer renderer(QString(":/icons/coverimage.svg"));

//            QRect rect;
//            rect.setSize(renderer.defaultSize().scaled(ui->mediaLibraryInfoWidget->width(), ui->mediaLibraryInfoWidget->height(), Qt::KeepAspectRatioByExpanding));
//            rect.setLeft(ui->mediaLibraryInfoWidget->width() / 2 - rect.width() / 2);
//            rect.setTop(ui->mediaLibraryInfoWidget->height() / 2 - rect.height() / 2);

//            renderer.render(&painter, rect);

//            painter.setBrush(backgroundCol);
//            painter.setPen(Qt::transparent);
//            painter.drawRect(0, 0, ui->mediaLibraryInfoWidget->width(), ui->mediaLibraryInfoWidget->height());
//            ui->buttonSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Preferred);
            ui->buttonSpacer->setFixedWidth(0);
        } else {
            QRect rect;
            rect.setSize(d->playlistBackground.size().scaled(ui->topWidget->width(), ui->topWidget->height(), Qt::KeepAspectRatioByExpanding));
            rect.moveLeft(ui->topWidget->width() / 2 - rect.width() / 2);
            rect.moveTop(ui->topWidget->height() / 2 - rect.height() / 2);

            //Blur the background
            paintCalculator.addRect(rect, [ = ](QRectF paintBounds) {
                int radius = 30;
                QGraphicsBlurEffect* blur = new QGraphicsBlurEffect;
                blur->setBlurRadius(radius);

                QGraphicsScene scene;
                QGraphicsPixmapItem item;
                item.setPixmap(QPixmap::fromImage(d->playlistBackground));
                item.setGraphicsEffect(blur);
                scene.addItem(&item);

                scene.render(painter, paintBounds.adjusted(-radius, -radius, radius, radius), QRectF(-radius, -radius, d->playlistBackground.width() + radius, d->playlistBackground.height() + radius));
            });


            QRect rightRect;
            rightRect.setSize(d->playlistBackground.size().scaled(0, ui->topWidget->height() - d->topPadding, Qt::KeepAspectRatioByExpanding));
            rightRect.moveRight(ui->topWidget->width());
            rightRect.moveTop(d->topPadding + (ui->topWidget->height() - d->topPadding) / 2 - rightRect.height() / 2);

            paintCalculator.addRect(rightRect, [ = ](QRectF paintBounds) {
                painter->setBrush(backgroundCol);
                painter->setPen(Qt::transparent);
                painter->drawRect(0, 0, ui->topWidget->width(), ui->topWidget->height());
                painter->drawImage(paintBounds, d->playlistBackground.scaled(rightRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            });

            ui->buttonSpacer->setFixedWidth(rightRect.width());
        }

        paintCalculator.performPaint();

        delete painter;
    }
    return false;
}

void ArtistsAlbumsWidget::on_initialList_itemActivated(QListWidgetItem* item) {
    d->listName = item->text();
    ui->tracksTitle->setText(d->type == Albums ? tr("Tracks in %1").arg(QLocale().quoteString(item->text())) : tr("Tracks by %1").arg(QLocale().quoteString(item->text())));
    LibraryModel* model = d->type == Albums ? LibraryManager::instance()->tracksByAlbum(item->text()) : LibraryManager::instance()->tracksByArtist(item->text());
    ui->tracksList->setModel(model);
    ui->stackedWidget->setCurrentWidget(ui->tracksPage);

    d->playlistBackground = model->index(0, 0).data(LibraryModel::AlbumArtRole).value<QImage>();
    ui->topWidget->update();

    QStringList parts;
    parts.append(tr("%n tracks", nullptr, model->rowCount()));

    quint64 duration = 0;
    for (int i = 0; i < model->rowCount(); i++) {
        duration += model->index(i, 0).data(LibraryModel::DurationRole).toInt();
    }
    parts.append(Common::durationToString(duration));

    ui->auxiliaryDataLabel->setText(parts.join(" · "));
}

void ArtistsAlbumsWidget::on_backButton_clicked() {
    ui->stackedWidget->setCurrentWidget(ui->mainPage);
}

void ArtistsAlbumsWidget::on_enqueueAllButton_clicked() {
    for (int i = 0; i < ui->tracksList->model()->rowCount(); i++) {
        if (ui->tracksList->model()->index(i, 0).data(LibraryModel::ErrorRole).value<LibraryModel::Errors>() != LibraryModel::NoError) continue;

        MediaItem* item = StateManager::instance()->url()->itemForUrl(QUrl::fromLocalFile(ui->tracksList->model()->index(i, 0).data(LibraryModel::PathRole).toString()));
        StateManager::instance()->playlist()->addItem(item);
    }
}

void ArtistsAlbumsWidget::on_shuffleButton_clicked() {
    ui->enqueueAllButton->click();
    StateManager::instance()->playlist()->setShuffle(true);
    StateManager::instance()->playlist()->next();
}

void ArtistsAlbumsWidget::on_playAllButton_clicked() {
    StateManager::instance()->playlist()->clear();
    ui->enqueueAllButton->click();
}

void ArtistsAlbumsWidget::on_burnButton_clicked() {
    QStringList files;
    for (int i = 0; i < ui->tracksList->model()->rowCount(); i++) {
        if (ui->tracksList->model()->index(i, 0).data(LibraryModel::ErrorRole).value<LibraryModel::Errors>() != LibraryModel::NoError) {
            //TODO: Do something!!!
            continue;
        }

        files.append(ui->tracksList->model()->index(i, 0).data(LibraryModel::PathRole).toString());
    }

    Common::showBurnMenu(files, d->listName, ui->burnButton);
}
