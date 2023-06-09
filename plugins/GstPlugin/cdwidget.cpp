#include "cdwidget.h"
#include "playlist.h"
#include "ui_cdwidget.h"

#include <DriveObjects/blockinterface.h>
#include <DriveObjects/diskobject.h>
#include <DriveObjects/driveinterface.h>
#include <mediaitem/gstcdplayback.h>
#include <pluginmediasource.h>
#include <sourcemanager.h>
#include <statemanager.h>

struct CdWidgetPrivate {
        PluginMediaSource* source;
        DiskObject* disk;
};

CdWidget::CdWidget(DiskObject* disk, QWidget* parent) :
    AbstractLibraryBrowser(parent),
    ui(new Ui::CdWidget) {
    ui->setupUi(this);
    d = new CdWidgetPrivate();
    d->disk = disk;

    d->source = new PluginMediaSource(this, this);
    d->source->setName(tr("CD"));
    d->source->setIcon(QIcon::fromTheme("media-optical-audio"));
    ui->albumTitleLabel->setText(tr("CD"));

    ui->topWidget->setContentsMargins(0, StateManager::instance()->sources()->padTop(), 0, 0);

    StateManager::instance()->sources()->addSource(d->source);

    ui->importCdButton->setVisible(false);
    ui->musicBrainzWidget->setVisible(false);
    updateTracks();
}

CdWidget::~CdWidget() {
    StateManager::instance()->sources()->removeSource(d->source);
    delete ui;
    delete d;
}

void CdWidget::updateTracks() {
    auto drive = d->disk->interface<BlockInterface>()->drive();
    ui->tracksWidget->clear();
    for (auto i = 0; i < drive->audioTracks(); i++) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(tr("Track %1").arg(i + 1));
        item->setData(Qt::UserRole, i + 1);
        ui->tracksWidget->addItem(item);
    }
}

AbstractLibraryBrowser::ListInformation CdWidget::currentListInformation() {
    return {};
}

void CdWidget::on_ejectButton_clicked() {
    d->disk->interface<BlockInterface>()->drive()->eject();
}

void CdWidget::on_playAllButton_clicked() {
    StateManager::instance()->playlist()->clear();
    ui->enqueueAllButton->click();
}

void CdWidget::on_shuffleAllButton_clicked() {
    ui->enqueueAllButton->click();
    StateManager::instance()->playlist()->setShuffle(true);
    StateManager::instance()->playlist()->next();
}

void CdWidget::on_tracksWidget_itemActivated(QListWidgetItem* item) {
    int track = item->data(Qt::UserRole).toInt();
    StateManager::instance()->playlist()->addItem(new GstCdPlayback(d->disk->interface<BlockInterface>()->blockName(), track));
}

void CdWidget::on_enqueueAllButton_clicked() {
    auto drive = d->disk->interface<BlockInterface>()->drive();
    for (auto i = 0; i < drive->audioTracks(); i++) {
        StateManager::instance()->playlist()->addItem(new GstCdPlayback(d->disk->interface<BlockInterface>()->blockName(), i + 1));
    }
}
