#ifndef GSTCDPLAYBACK_H
#define GSTCDPLAYBACK_H

#include "../gsttrackinfo.h"
#include "gstmediaitem.h"
#include <QObject>

struct GstCdPlaybackPrivate;
class GstCdPlayback : public GstMediaItem {
        Q_OBJECT
    public:
        GstCdPlayback(QString device, int track, GstTrackInfoPtr trackInfo);
        ~GstCdPlayback();

    private:
        GstCdPlaybackPrivate* d;

        // GstMediaItem interface
    protected:
        GstElement* pipeline();

        // MediaItem interface
    public:
        QString title();
        QStringList authors();
        QString album();
        void preparePlayer();
        QVariant metadata(QString key);
};

#endif // GSTCDPLAYBACK_H
