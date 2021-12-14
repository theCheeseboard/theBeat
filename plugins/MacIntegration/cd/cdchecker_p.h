#ifndef CDCHECKER_P_H
#define CDCHECKER_P_H

#include "pluginmediasource.h"
#include "trackinfo.h"
#include <QNetworkAccessManager>

#ifdef HAVE_MUSICBRAINZ
    #include <musicbrainz5/ReleaseList.h>
#endif

struct CdCheckerPrivate {
    QString directory;

    PluginMediaSource* source;
    QString mbDiscId;
    QString albumName;
    QList<TrackInfoPtr> trackInfo;

    QImage playlistBackground;
    QNetworkAccessManager mgr;

#ifdef HAVE_MUSICBRAINZ
    QString currentDiscId;
    QString currentReleaseId;
    MusicBrainz5::CReleaseList releases;
#endif
};

#endif // CDCHECKER_P_H
