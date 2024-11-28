#ifndef MUSICBRAINZCLIENT_H
#define MUSICBRAINZCLIENT_H

#include <QCoroTask>
#include <QObject>

struct MusicBrainzClientPrivate;
class MusicBrainzClient : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString albumName READ albumName NOTIFY albumNameChanged FINAL)
        Q_PROPERTY(QImage albumArt READ albumArt NOTIFY albumArtChanged FINAL)
    public:
        explicit MusicBrainzClient(int firstTrack, int lastTrack, int leadOut, int frameOffsets[99], QObject* parent = nullptr);
        ~MusicBrainzClient();

        struct MusicBrainzTrack {
                QString title;
                QStringList artists;
                QString album;
        };

        QString albumName();
        QImage albumArt();
        MusicBrainzTrack track(int index);
        int trackCount();

        QCoro::Task<> selectMusicbrainzRelease(QString release);

    signals:
        void albumNameChanged();
        void albumArtChanged();
        void tracksChanged();

    private:
        MusicBrainzClientPrivate* d;

        QCoro::Task<> loadMusicbrainzData();
};

#endif // MUSICBRAINZCLIENT_H
