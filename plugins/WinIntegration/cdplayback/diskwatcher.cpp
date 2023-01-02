#include "diskwatcher.h"

#include "cdchecker.h"
#include <tpromise.h>
#include <tlogger.h>
#include <QTimer>
#include "audiocdplayerthread.h"

#include <Windows.h>
#include <winrt/CDLib.h>
#include <winrt/Windows.Foundation.Collections.h>

struct DiskWatcherPrivate {
    QMap<QChar, CdChecker*> cdCheckers;
};

DiskWatcher::DiskWatcher(QObject* parent) : QObject(parent) {
    d = new DiskWatcherPrivate();
    tDebug("DiskWatcher") << "Ready";

    AudioCdPlayerThread::staticMetaObject.invokeMethod(AudioCdPlayerThread::instance(), &AudioCdPlayerThread::start, Qt::QueuedConnection);
    connect(AudioCdPlayerThread::instance(), &AudioCdPlayerThread::ready, this, &DiskWatcher::checkCd);
}

DiskWatcher::~DiskWatcher() {
    delete d;
}

void DiskWatcher::checkCd() {
    struct CdInformation {
        QList<QChar> availableDriveLetters;
    };
    tDebug("DiskWatcher") << "Checking CDs";

    AudioCdPlayerThread* thread = AudioCdPlayerThread::instance();
    auto drives = thread->player().GetDrives();
    CdInformation info;
    for (uint i = 0; i < drives.Size(); i++) {
        auto letter = drives.GetAt(i).DriveLetter();
        info.availableDriveLetters.append(letter.Value());
    }
    for (QChar letter : info.availableDriveLetters) {
        tDebug("DiskWatcher") << "Found CD " << letter;
        if (!d->cdCheckers.contains(letter)) d->cdCheckers.insert(letter, new CdChecker(letter));
    }
}
