#ifndef GSTCDPLAYBACK_H
#define GSTCDPLAYBACK_H

#include "gstmediaitem.h"
#include <QObject>

struct GstCdPlaybackPrivate;
class GstCdPlayback : public GstMediaItem {
        Q_OBJECT
    public:
        GstCdPlayback(QString device, int track);
        ~GstCdPlayback();

    private:
        GstCdPlaybackPrivate* d;

        // GstMediaItem interface
    protected:
        GstElement* pipeline();

        // MediaItem interface
    public:
        QString title();
        void preparePlayer();
};

#endif // GSTCDPLAYBACK_H
