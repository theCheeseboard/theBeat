#ifndef CDCHECKER_P_H
#define CDCHECKER_P_H

#include "pluginmediasource.h"
#include "trackinfo.h"
#include <QNetworkAccessManager>

struct CdCheckerPrivate {
    QString directory;

    PluginMediaSource* source;
    QStringList mbDiscIds;
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
