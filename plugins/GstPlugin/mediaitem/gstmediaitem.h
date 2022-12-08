#ifndef GSTMEDIAITEM_H
#define GSTMEDIAITEM_H

#include <QObject>
#include <gst/gst.h>
#include <mediaitem.h>

struct GstMediaItemPrivate;
class GstMediaItem : public MediaItem {
        Q_OBJECT
    public:
        GstMediaItem();
        ~GstMediaItem();

    protected:
        virtual GstElement* pipeline() = 0;
        virtual void preparePlayer();

    private:
        GstMediaItemPrivate* d;

        // MediaItem interface
    public:
        void play();
        void pause();
        void stop();
        void seek(quint64 ms);
        quint64 elapsed();
        quint64 duration();
        QString title();
        QStringList authors();
        QString album();
        QImage albumArt();
        QString lyrics();
        QString lyricFormat();
        QVariant metadata(QString key);
};

#endif // GSTMEDIAITEM_H
