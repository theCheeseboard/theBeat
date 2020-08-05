#include "currenttrackpopover.h"
#include "ui_currenttrackpopover.h"

#include <QPainter>
#include <QGraphicsBlurEffect>
#include <QMediaMetaData>
#include <statemanager.h>
#include <playlist.h>
#include <mediaitem.h>
#include <the-libs_global.h>

struct CurrentTrackPopoverPrivate {
    MediaItem* currentItem = nullptr;
    QLabel* artBackground;

    QList<QLabel*> metadataInfo;
    QString pendingMetadataTitle;
    int currentRow = 0;
};

CurrentTrackPopover::CurrentTrackPopover(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CurrentTrackPopover)
{
    ui->setupUi(this);
    d = new CurrentTrackPopoverPrivate();

    d->artBackground = new QLabel(this);
    d->artBackground->setScaledContents(true);
    d->artBackground->lower();

    QGraphicsBlurEffect* effect = new QGraphicsBlurEffect(d->artBackground);
    effect->setBlurRadius(50);
    d->artBackground->setGraphicsEffect(effect);

    ui->titleLabel->setFixedWidth(SC_DPI(300));

    connect(StateManager::instance()->playlist(), &Playlist::currentItemChanged, this, &CurrentTrackPopover::updateCurrentItem);
    updateCurrentItem();
}

CurrentTrackPopover::~CurrentTrackPopover()
{
    delete d;
    delete ui;
}

void CurrentTrackPopover::updateCurrentItem()
{
    if (d->currentItem) {
        d->currentItem->disconnect(this);
    }

    d->currentItem = StateManager::instance()->playlist()->currentItem();
    if (d->currentItem) {
        connect(d->currentItem, &MediaItem::metadataChanged, this, &CurrentTrackPopover::updateMetadata);
        updateMetadata();
    }
}

void CurrentTrackPopover::updateMetadata()
{
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
        d->artBackground->setVisible(false);
    } else {
        QPixmap art = QPixmap::fromImage(image).scaledToWidth(SC_DPI(300));
        ui->artLabel->setPixmap(art);
        ui->artLabel->setVisible(true);

        QPixmap transparentArt = art;
        QPainter painter(&transparentArt);
        painter.setOpacity(0.5);
        painter.setPen(Qt::transparent);
        painter.setBrush(this->palette().color(QPalette::Window));
        painter.drawRect(0, 0, art.width(), art.height());
        d->artBackground->setPixmap(transparentArt);
        d->artBackground->setVisible(true);
    }

    QLocale locale;
    clearMetadataInfo();
    addMetadataTitle(tr("Album"));
    addMetadataEntry(tr("Name"), d->currentItem->metadata(QMediaMetaData::AlbumTitle).toString());

    int track = d->currentItem->metadata(QMediaMetaData::TrackNumber).toInt();
    int totalTrack = d->currentItem->metadata(QMediaMetaData::TrackCount).toInt();
    if (track > 0 && totalTrack > 0) {
        addMetadataEntry(tr("Track"), tr("%1 of %2", "Track 1 of 12").arg(track).arg(totalTrack));
    } else if (track > 0) {
        addMetadataEntry(tr("Track"), locale.toString(track));
    }

    addMetadataTitle(tr("Track"));
    if (d->currentItem->metadata(QMediaMetaData::Year).toInt() > 0) addMetadataEntry(tr("Year"), QString::number(d->currentItem->metadata(QMediaMetaData::Year).toInt()));
}

void CurrentTrackPopover::addMetadataTitle(QString title)
{
    d->pendingMetadataTitle = title;
}

void CurrentTrackPopover::addMetadataEntry(QString entry, QString value)
{
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

    QLabel* valueLabel = new QLabel(this);
    valueLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    valueLabel->setText(value);
    ui->metadataEntryLayout->addWidget(valueLabel, d->currentRow, 1);

    d->currentRow++;
}

void CurrentTrackPopover::clearMetadataInfo()
{
    for (QLabel* label : d->metadataInfo) {
        ui->metadataEntryLayout->removeWidget(label);
        label->deleteLater();
    }
    d->metadataInfo.clear();
    d->currentRow = 0;
}

void CurrentTrackPopover::resizeEvent(QResizeEvent *event)
{
    updateMetadata();

    QRect geometry;
    geometry.setSize(QSize(1, 1).scaled(this->width(), this->height(), Qt::KeepAspectRatioByExpanding));
    geometry.moveCenter(QPoint(this->width() / 2, this->height() / 2));
    d->artBackground->setGeometry(geometry);
}
