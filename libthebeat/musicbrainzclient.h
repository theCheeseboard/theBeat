#ifndef MUSICBRAINZCLIENT_H
#define MUSICBRAINZCLIENT_H

#include <QAbstractListModel>
#include <QCoroTask>

struct MusicBrainzClientPrivate;
class MusicBrainzClient : public QAbstractListModel {
        Q_OBJECT
        Q_PROPERTY(QString albumName READ albumName NOTIFY albumNameChanged FINAL)
        Q_PROPERTY(QImage albumArt READ albumArt NOTIFY albumArtChanged FINAL)
        Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged FINAL)
        Q_PROPERTY(bool supported READ supported FINAL CONSTANT)
        Q_PROPERTY(QString selectedReleaseId READ selectedReleaseId NOTIFY selectedReleaseIdChanged FINAL)
    public:
        explicit MusicBrainzClient(int firstTrack, int lastTrack, int leadOut, int frameOffsets[99], QObject* parent = nullptr);
        ~MusicBrainzClient();

        struct MusicBrainzTrack {
                QString title;
                QStringList artists;
                QString album;
        };

        enum Role {
            ComboBoxLabel,
            ReleaseTitle,
            ReleaseCountry,
            ReleaseDate,
            ReleaseBarcode,
            ReleaseId
        };

        QString albumName();
        QImage albumArt();
        MusicBrainzTrack track(int index);
        int trackCount();
        bool loading();
        bool supported();
        QString selectedReleaseId();

        Q_SCRIPTABLE QCoro::Task<> selectMusicbrainzRelease(QString release);

    signals:
        void albumNameChanged();
        void albumArtChanged();
        void tracksChanged();
        void loadingChanged();
        void selectedReleaseIdChanged();

    private:
        MusicBrainzClientPrivate* d;

        QCoro::Task<> loadMusicbrainzData();

        // QAbstractItemModel interface
    public:
        int rowCount(const QModelIndex& parent) const;
        QVariant data(const QModelIndex& index, int role) const;
        QHash<int, QByteArray> roleNames() const;
};

#endif // MUSICBRAINZCLIENT_H
