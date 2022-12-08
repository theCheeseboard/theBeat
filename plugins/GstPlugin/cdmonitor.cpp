#include "cdmonitor.h"

#include "cdwidget.h"
#include <DriveObjects/blockinterface.h>
#include <DriveObjects/diskobject.h>
#include <DriveObjects/driveinterface.h>
#include <driveobjectmanager.h>
#include <statemanager.h>

struct CdMonitorPrivate {
        QMap<DiskObject*, CdWidget*> panes;
};

CdMonitor::CdMonitor(QObject* parent) :
    QObject{parent} {
    d = new CdMonitorPrivate();

    for (auto drive : DriveObjectManager::drives()) {
        connect(drive, &DriveInterface::changed, this, &CdMonitor::updateDisks);
    }
    connect(DriveObjectManager::instance(), &DriveObjectManager::driveAdded, this, [this](DriveInterface* drive) {
        connect(drive, &DriveInterface::changed, this, &CdMonitor::updateDisks);
    });
    updateDisks();
}

CdMonitor::~CdMonitor() {
    delete d;
}

void CdMonitor::updateDisks() {
    QList<DiskObject*> keepDisks;
    for (auto disk : DriveObjectManager::opticalDisks()) {
        auto drive = disk->interface<BlockInterface>()->drive();
        if (!drive->mediaAvailable()) continue;
        if (drive->audioTracks() == 0) continue;

        if (!d->panes.contains(disk)) {
            auto widget = new CdWidget(disk);
            d->panes.insert(disk, widget);
        }

        keepDisks.append(disk);
    }

    auto oldDisks = d->panes.keys();
    for (auto disk : oldDisks) {
        if (!keepDisks.contains(disk)) {
            auto widget = d->panes.take(disk);
            widget->deleteLater();
        }
    }
}
