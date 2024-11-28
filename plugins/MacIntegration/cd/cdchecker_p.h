#ifndef CDCHECKER_P_H
#define CDCHECKER_P_H

#include "musicbrainzclient.h"
#include "pluginmediasource.h"
#include "trackinfo.h"
#include <QNetworkAccessManager>

struct CdCheckerPrivate {
        QString directory;

        PluginMediaSource* source;
        QString albumName;
        QList<TrackInfoPtr> trackInfo;

        QImage playlistBackground;
        QNetworkAccessManager mgr;

        MusicBrainzClient* musicBrainzClient = nullptr;
};

#endif // CDCHECKER_P_H
