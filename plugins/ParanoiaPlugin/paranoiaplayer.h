#ifndef PARANOIAPLAYER_H
#define PARANOIAPLAYER_H

#include <QObject>
#include <cdio++/cdio.hpp>

struct ParanoiaPlayerPrivate;
class ParanoiaPlayer : public QObject {
        Q_OBJECT
    public:
        explicit ParanoiaPlayer(CdioDevice* cdio, QObject* parent = nullptr);
        ~ParanoiaPlayer();

        struct Track {
                lsn_t firstLsn;
                lsn_t lastLsn;
        };

        QByteArray nextFrame(int frameCount);
        bool isFrameAvailable();

        ParanoiaPlayer::Track track(int track);

        void seek(int track, quint64 ms);

    private slots:
        void handleFrameRead(lba_t position);

    signals:
        void frameAvailable();
        void epochChanged();
        void frameRead(lsn_t position);

    private:
        ParanoiaPlayerPrivate* d;
};

struct ParanoiaPlayerWorkerPrivate;
class ParanoiaPlayerWorker : public QObject {
        Q_OBJECT
    public:
        explicit ParanoiaPlayerWorker(CdioDevice* cdio);
        ~ParanoiaPlayerWorker();

        QByteArray nextFrame(int frameCount);
        bool isFrameAvailable();

        ParanoiaPlayer::Track track(int track);
        lsn_t position();

    signals:
        void frameAvailable();
        void epochChanged();
        void frameRead(lsn_t position);

    public slots:
        void prepare();
        void seek(int track, quint64 ms);

    private slots:
        void tryReadNextFrame();
        void bumpEpoch();

    private:
        ParanoiaPlayerWorkerPrivate* d;
};

#endif // PARANOIAPLAYER_H
