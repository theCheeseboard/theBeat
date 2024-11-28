#include "cdchecker.h"
#include "cdchecker_p.h"

#import <AppKit/AppKit.h>
#include <IOKit/IOKitLib.h>

#include <QDir>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QCoroFuture>
#include <QtConcurrent>

#include "maccdmediaitem.h"
#include <tlogger.h>
#include <tpromise.h>
#include <tmessagebox.h>
#include <statemanager.h>

QCoro::Task<> CdChecker::eject()
{
    MacCdMediaItem::volumeGone(d->directory);

    auto result = co_await QtConcurrent::run([](QString directory) {
        NSError* error;
        BOOL ejected = [[NSWorkspace sharedWorkspace] unmountAndEjectDeviceAtURL:QUrl::fromLocalFile(directory).toNSURL() error:&error];

        return ejected == YES;
    }, d->directory);

    if (!result) {
       emit ejectError();
    }
}

void CdChecker::setupMusicBrainzClient() {
    QDir dir(d->directory);

    NSError* error;
    NSDictionary* dict = [NSDictionary dictionaryWithContentsOfFile:dir.absoluteFilePath(".TOC.plist").toNSString()];
    NSData* jsonData = [NSJSONSerialization dataWithJSONObject:dict[@"Sessions"] options:0 error:&error];
    if (!jsonData) return; //Bail out

    QByteArray json = QByteArray::fromNSData(jsonData);
    QJsonArray sessions = QJsonDocument::fromJson(json).array();
    if (sessions.count() != 1) return;

    QJsonObject session = sessions.at(0).toObject();
    QJsonArray tracks = session.value("Track Array").toArray();

    int frameOffsets[99];
    for (int i = 0; i < 99; i++) {
        int frameOffset = 0;
        if (i < tracks.count()) {
            auto track = tracks.at(i).toObject();
            frameOffset = track.value("Start Block").toInt();
        }
        frameOffsets[i] = frameOffset;
    }

    d->musicBrainzClient = new MusicBrainzClient(session.value("First Track").toInt(), session.value("Last Track").toInt(), session.value("Leadout Block").toInt(), frameOffsets, this);
}
