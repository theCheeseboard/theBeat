#include "diskwatcher.h"

#include <tlogger.h>
#include <QDir>
#include <QFileSystemWatcher>
#include "cdchecker.h"

struct DiskWatcherPrivate {
    QFileSystemWatcher* volumesWatcher;
    QMap<QString, CdChecker*> checkers;
};

DiskWatcher::DiskWatcher(QObject* parent) : QObject(parent) {
    d = new DiskWatcherPrivate();

    d->volumesWatcher = new QFileSystemWatcher(this);
    d->volumesWatcher->addPath(QStringLiteral("/Volumes/"));
    connect(d->volumesWatcher, &QFileSystemWatcher::directoryChanged, this, &DiskWatcher::scanDisks);

    scanDisks();
}

DiskWatcher::~DiskWatcher() {
    delete d;
}

void DiskWatcher::scanDisks() {
    tDebug("DiskWatcher") << "Scanning disks";

    QDir dir("/Volumes");
    const QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (QString entry : entries) {
        QDir directory(dir.absoluteFilePath(entry));
        bool isCd = QFile::exists(directory.absoluteFilePath(".TOC.plist"));
        if (isCd && !d->checkers.contains(entry)) {
            d->checkers.insert(entry, new CdChecker(directory.path()));
        } else if (!isCd && d->checkers.contains(entry)) {
            d->checkers.take(entry)->deleteLater();
        }
    }

    //Look for volumes that no longer exist
    QStringList gone;
    for (QString entry : d->checkers.keys()) {
        if (!entries.contains(entry)) gone.append(entry);
    }

    for (QString entry : gone) {
        d->checkers.take(entry)->deleteLater();
    }
}
