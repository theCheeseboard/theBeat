#include "cdchecker.h"
#include "cdchecker_p.h"

#import <AppKit/AppKit.h>
#include <IOKit/IOKitLib.h>

#include <QDir>
#include <QJsonDocument>
#include <QCryptographicHash>

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
        warning->setTitleBarText(tr("Couldn't eject the disc"));
        warning->setMessageText(tr("Make sure no other applications are accessing the disc, and then try again."));
        warning->setIcon(QMessageBox::Warning);
        warning->show(true);
    });
}

QString CdChecker::calculateMbDiscId() {
    QDir dir(d->directory);

    NSError* error;
    NSDictionary* dict = [NSDictionary dictionaryWithContentsOfFile:dir.absoluteFilePath(".TOC.plist").toNSString()];
    NSData* jsonData = [NSJSONSerialization dataWithJSONObject:dict[@"Sessions"] options:0 error:&error];
    if (!jsonData) return ""; //Bail out

    QByteArray json = QByteArray::fromNSData(jsonData);
    QJsonArray sessions = QJsonDocument::fromJson(json).array();
    if (sessions.count() != 1) return "";

    QJsonObject session = sessions.at(0).toObject();

    QString data;
    data.append(QString::asprintf("%02X", session.value("First Track").toInt()));
    data.append(QString::asprintf("%02X", session.value("Last Track").toInt()));
    data.append(QString::asprintf("%08X", session.value("Leadout Block").toInt()));

    QJsonArray tracks = session.value("Track Array").toArray();

    for (int i = 0; i < 99; i++) {
        int frameOffset = 0;
        if (i < tracks.count()) {
            QJsonObject track = tracks.at(i).toObject();
            frameOffset = track.value("Start Block").toInt();
        }
        data.append(QString::asprintf("%08X", frameOffset));
    }

    QByteArray hash = QCryptographicHash::hash(data.toLatin1(), QCryptographicHash::Sha1);
    QString formatted = hash.toBase64(QByteArray::Base64Encoding).replace("+", ".").replace("/", "_").replace("=", "-");
    tDebug("CdChecker") << "MusicBrainz Disc Id: " << formatted;
    return formatted;
}
