#ifndef PARANOIAMEDIAITEM_H
#define PARANOIAMEDIAITEM_H

#include "paranoiatrackinfo.h"
#include <QObject>
#include <cdio++/cdio.hpp>
#include <mediaitem.h>

class QAudioSink;
class ParanoiaPlayer;
struct ParanoiaMediaItemPrivate;
class ParanoiaMediaItem : public MediaItem {
        Q_OBJECT
    public:
        explicit ParanoiaMediaItem(ParanoiaPlayer* player, int track, QAudioSink* sink, ParanoiaTrackInfoPtr trackInfo);
        ~ParanoiaMediaItem();

    signals:

    private slots:
        void frameRead(lsn_t position);

    private:
        ParanoiaMediaItemPrivate* d;

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

#endif // PARANOIAMEDIAITEM_H
