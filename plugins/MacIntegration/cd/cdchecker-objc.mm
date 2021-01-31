#include "cdchecker.h"
#include "cdchecker_p.h"

#import <AppKit/AppKit.h>

#include "maccdmediaitem.h"
#include <tlogger.h>
#include <tpromise.h>
#include <tmessagebox.h>
#include <statemanager.h>

void CdChecker::on_ejectButton_clicked() {
    MacCdMediaItem::volumeGone(d->directory);

    TPROMISE_CREATE_NEW_THREAD(void, {
        NSError* error;
        BOOL ejected = [[NSWorkspace sharedWorkspace] unmountAndEjectDeviceAtURL:QUrl::fromLocalFile(d->directory).toNSURL() error:&error];

        if (ejected == NO) {
            rej(QString::fromNSString([error description]));
        } else {
            res();
        }
    })->error([ = ](QString error) {
        tMessageBox* warning = new tMessageBox(StateManager::instance()->mainWindow());
        warning->setWindowTitle(tr("Couldn't eject the disc"));
        warning->setText(tr("Make sure no other applications are accessing the disc, and then try again."));
        warning->setIcon(tMessageBox::Warning);
        warning->setWindowFlag(Qt::Sheet);
        connect(warning, &tMessageBox::finished, warning, &tMessageBox::deleteLater);
        warning->open();
    });
}
