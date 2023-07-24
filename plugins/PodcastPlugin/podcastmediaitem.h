#ifndef PODCASTMEDIAITEM_H
#define PODCASTMEDIAITEM_H

#include "podcastitem.h"
#include <QObject>
#include <dependencyinjection/tinjectedpointer.h>
#include <iurlmanager.h>
#include <mediaitem.h>

struct PodcastMediaItemPrivate;
class PodcastMediaItem : public MediaItem {
        Q_OBJECT
    public:
        PodcastMediaItem(PodcastItemPtr podcastItem, T_INJECT(IUrlManager));
        ~PodcastMediaItem();

    private:
        PodcastMediaItemPrivate* d;

        void updatePodcastElapsed();

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
};

#endif // PODCASTMEDIAITEM_H
