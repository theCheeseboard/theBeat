#include "currenttrackpopover.h"
#include "ui_currenttrackpopover.h"

#include <QPainter>
#include <QGraphicsBlurEffect>
#include <QMediaMetaData>
#include <statemanager.h>
#include <playlist.h>
#include <sourcemanager.h>
#include <mediaitem.h>
#include <tvariantanimation.h>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <visualisationmanager.h>
#include <tcsdtools.h>
#include "common.h"
#include "lyrics/abstractlyricformat.h"

struct CurrentTrackPopoverPrivate {
    MediaItem* currentItem = nullptr;
    QImage artBackground;

    QList<QLabel*> metadataInfo;
    QString pendingMetadataTitle;
    int currentRow = 0;

    tCsdTools csd;
    QWidget* backing = nullptr;
    QWidget* windowDragWidget = nullptr;
};

CurrentTrackPopover::CurrentTrackPopover(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::CurrentTrackPopover) {
    ui->setupUi(this);
    d = new CurrentTrackPopoverPrivate();

    ui->titleLabel->setFixedWidth(SC_DPI(300));
    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Fade);

    connect(StateManager::instance()->playlist(), &Playlist::currentItemChanged, this, &CurrentTrackPopover::updateCurrentItem);
    connect(StateManager::instance()->playlist(), &Playlist::stateChanged, this, &CurrentTrackPopover::updateState);
    updateCurrentItem();
    updateState();
    updateRightPane(true);

    QSize iconSize = SC_DPI_T(QSize(32, 32), QSize);
    QSize bigIconSize = SC_DPI_T(QSize(64, 64), QSize);

    ui->skipBackButton->setIconSize(iconSize);
    ui->playButton->setIconSize(bigIconSize);
    ui->skipNextButton->setIconSize(iconSize);

    connect(StateManager::instance()->visualisation(), &VisualisationManager::visualisationUpdated, this, [ = ] {
        this->update();
    });

    //Ensure that the seeker and transport controls are always LTR
    ui->transportControlsWidget->setLayoutDirection(Qt::LeftToRight);
    ui->seekerWidget->setLayoutDirection(Qt::LeftToRight);

    if (this->layoutDirection() == Qt::RightToLeft) {
        ui->titleLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->metadataLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }

    QPalette pal = ui->stackedWidget->palette();
    pal.setColor(QPalette::Window, Qt::transparent);
    ui->stackedWidget->setPalette(pal);
}

CurrentTrackPopover::~CurrentTrackPopover() {
    delete d;
    delete ui;
}

void CurrentTrackPopover::setBacking(QWidget* backing) {
    d->backing = backing;
    this->window()->installEventFilter(this);

    d->windowDragWidget = new QWidget();
    d->windowDragWidget->setParent(this);
    d->windowDragWidget->move(0, 0);
    d->windowDragWidget->resize(d->backing->width(), StateManager::instance()->sources()->padTop());
    d->windowDragWidget->show();

    d->csd.installMoveAction(d->windowDragWidget);
}

void CurrentTrackPopover::updateCurrentItem() {
    if (d->currentItem) {
        d->currentItem->disconnect(this);
    }

    d->currentItem = StateManager::instance()->playlist()->currentItem();
    if (d->currentItem) {
        connect(d->currentItem, &MediaItem::metadataChanged, this, &CurrentTrackPopover::updateMetadata);
        connect(d->currentItem, &MediaItem::elapsedChanged, this, &CurrentTrackPopover::updateBar);
        connect(d->currentItem, &MediaItem::elapsedChanged, this, [=] {
            this->updateRightPane(false);
        });
        connect(d->currentItem, &MediaItem::durationChanged, this, &CurrentTrackPopover::updateBar);

        ui->lyricsWidget->setLyrics(AbstractLyricFormat::loadLyricFile(d->currentItem->lyricFormat(), d->currentItem->lyrics()));

        updateMetadata();
        updateBar();
    }
}

void CurrentTrackPopover::updateMetadata() {
    if (!d->currentItem) return;
    ui->titleLabel->setText(d->currentItem->title());

    QStringList parts;
    parts.append(QLocale().createSeparatedList(d->currentItem->authors()));
    parts.append(d->currentItem->album());

    parts.removeAll("");
    ui->metadataLabel->setText(parts.join(" · "));

    QImage image = d->currentItem->albumArt();
    if (image.isNull()) {
        ui->artLabel->setVisible(false);
        d->artBackground.fill(this->palette().color(QPalette::Window));
    } else {
        QPixmap art = QPixmap::fromImage(image).scaledToWidth(SC_DPI(300), Qt::SmoothTransformation);
        ui->artLabel->setPixmap(art);
        ui->artLabel->setVisible(true);

        //Blur the background
        int radius = 30;
        QGraphicsBlurEffect* blur = new QGraphicsBlurEffect;
        blur->setBlurRadius(radius);

        d->artBackground = QImage(image.size(), QImage::Format_ARGB32);
        d->artBackground.fill(this->palette().color(QPalette::Window));
        QPainter painter(&d->artBackground);

        QGraphicsScene scene;
        QGraphicsPixmapItem item;
        item.setPixmap(QPixmap::fromImage(image));
        item.setGraphicsEffect(blur);
        scene.addItem(&item);

        scene.render(&painter);
    }

    QLocale locale;
    clearMetadataInfo();
    addMetadataTitle(tr("Album"));
    addMetadataEntry(tr("Name"), d->currentItem->metadata(QMediaMetaData::AlbumTitle).toString());

    int track = d->currentItem->metadata(QMediaMetaData::TrackNumber).toInt();
        addMetadataEntry(tr("Track"), locale.toString(track));

    addMetadataTitle(tr("Track"));

    //TODO: Year Metadata
//    if (d->currentItem->metadata(QMediaMetaData::Year).toInt() > 0) addMetadataEntry(tr("Year"), QString::number(d->currentItem->metadata(QMediaMetaData::Year).toInt()));

    this->update();
}

void CurrentTrackPopover::addMetadataTitle(QString title) {
    d->pendingMetadataTitle = title;
}

void CurrentTrackPopover::addMetadataEntry(QString entry, QString value) {
    if (value.isEmpty()) return;

    if (!d->pendingMetadataTitle.isEmpty()) {
        QLabel* title = new QLabel(this);
        title->setText(d->pendingMetadataTitle.toUpper());

        QFont fnt = title->font();
        fnt.setBold(true);
        title->setFont(fnt);

        d->metadataInfo.append(title);
        ui->metadataEntryLayout->addWidget(title, d->currentRow, 0, 1, 0);
        d->pendingMetadataTitle.clear();
        d->currentRow++;
    }

    QLabel* entryLabel = new QLabel(this);
    entryLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    entryLabel->setText(entry);
    ui->metadataEntryLayout->addWidget(entryLabel, d->currentRow, 0);
    d->metadataInfo.append(entryLabel);

    QLabel* valueLabel = new QLabel(this);
    valueLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    valueLabel->setText(value);
    ui->metadataEntryLayout->addWidget(valueLabel, d->currentRow, 1);
    d->metadataInfo.append(valueLabel);

    if (this->layoutDirection() == Qt::RightToLeft) {
        entryLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }

    d->currentRow++;
}

void CurrentTrackPopover::clearMetadataInfo() {
    for (QLabel* label : qAsConst(d->metadataInfo)) {
        ui->metadataEntryLayout->removeWidget(label);
        label->deleteLater();
    }
    d->metadataInfo.clear();
    d->currentRow = 0;
}

void CurrentTrackPopover::updateState() {
    switch (StateManager::instance()->playlist()->state()) {
        case Playlist::Playing:
            ui->playButton->setIcon(QIcon::fromTheme("media-playback-pause"));
            break;
        case Playlist::Paused:
        case Playlist::Stopped:
            ui->playButton->setIcon(QIcon::fromTheme("media-playback-start"));
            break;
    }
}

void CurrentTrackPopover::updateBar() {
    QSignalBlocker blocker(ui->progressSlider);
    ui->progressSlider->setMaximum(d->currentItem->duration());
    ui->progressSlider->setValue(d->currentItem->elapsed());

    ui->durationLabel->setText(Common::durationToString(d->currentItem->duration(), true));
    ui->elapsedLabel->setText(Common::durationToString(d->currentItem->elapsed()));
}

void CurrentTrackPopover::updateRightPane(bool initial)
{
    if (d->currentItem) {
        if (d->currentItem->elapsed() > 5000 && !d->currentItem->lyrics().isEmpty()) {
            if (ui->stackedWidget->currentWidget() != ui->lyricsPage) ui->stackedWidget->setCurrentWidget(ui->lyricsPage, !initial);

            ui->lyricsWidget->setTime(d->currentItem->elapsed());
        } else if (ui->stackedWidget->currentWidget() != ui->trackInfoPage) {
            ui->stackedWidget->setCurrentWidget(ui->trackInfoPage, !initial);
        }
    }
}

void CurrentTrackPopover::resizeEvent(QResizeEvent* event) {
    updateMetadata();

    QRect geometry;
    geometry.setSize(QSize(1, 1).scaled(this->width(), this->height(), Qt::KeepAspectRatioByExpanding));
    geometry.moveCenter(QPoint(this->width() / 2, this->height() / 2));
//    d->artBackground->setGeometry(geometry);
}

void CurrentTrackPopover::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setOpacity(0.7);

    //Draw art background
    QRect geometry;
    geometry.setSize(d->artBackground.size().scaled(this->width(), this->height(), Qt::KeepAspectRatioByExpanding));
    geometry.moveCenter(QPoint(this->width() / 2, this->height() / 2));
    painter.drawImage(geometry, d->artBackground);

    painter.setOpacity(1);

    //Draw visualisations
    QRect visRect;
    visRect.setSize(QSize(this->width(), this->height() * 0.6));
    visRect.moveLeft(0);
    visRect.moveBottom(this->height());
    StateManager::instance()->visualisation()->paint(&painter, this->palette().color(QPalette::WindowText), visRect);
}

bool CurrentTrackPopover::eventFilter(QObject* watched, QEvent* event) {
    if (watched == this->window()) {
        if (event->type() == QEvent::Resize) {
            d->backing->resize(d->backing->parentWidget()->size());
            d->windowDragWidget->resize(d->backing->width(), StateManager::instance()->sources()->padTop());
        } else if (event->type() == QEvent::Close) {
            //Close zen mode
            QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(d->backing);
            effect->setOpacity(1);
            d->backing->setGraphicsEffect(effect);

            tVariantAnimation* opacityAnim = new tVariantAnimation(d->backing);
            opacityAnim->setStartValue(1.0);
            opacityAnim->setEndValue(0.0);
            opacityAnim->setDuration(500);
            opacityAnim->setEasingCurve(QEasingCurve::OutCubic);
            connect(opacityAnim, &tVariantAnimation::valueChanged, this, [ = ](QVariant value) {
                effect->setOpacity(value.toReal());
            });
            connect(opacityAnim, &tVariantAnimation::finished, this, [ = ] {
                d->backing->deleteLater();
            });
            opacityAnim->start();

            event->ignore();
            return true;
        }
    }
    return false;
}

void CurrentTrackPopover::on_skipBackButton_clicked() {
    StateManager::instance()->playlist()->previous();
}

void CurrentTrackPopover::on_playButton_clicked() {
    StateManager::instance()->playlist()->playPause();
}

void CurrentTrackPopover::on_skipNextButton_clicked() {
    StateManager::instance()->playlist()->next();
}

void CurrentTrackPopover::on_progressSlider_valueChanged(int value) {
    d->currentItem->seek(value);
}
