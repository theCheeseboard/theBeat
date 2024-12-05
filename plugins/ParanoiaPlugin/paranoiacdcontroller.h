#ifndef PARANOIACDCONTROLLER_H
#define PARANOIACDCONTROLLER_H

#include <QAbstractListModel>
#include <QCoroTask>
#include <QWidget>
#include <cdio++/cdio.hpp>
#include <musicbrainzclient.h>

class MediaItem;
struct ParanoiaCdControllerPrivate;
class ParanoiaCdController : public QAbstractListModel {
        Q_OBJECT
        Q_PROPERTY(QString albumName READ albumName NOTIFY albumNameChanged FINAL)
        Q_PROPERTY(MusicBrainzClient* musicBrainzClient READ musicBrainzClient NOTIFY musicBrainzClientChanged FINAL)

    public:
        explicit ParanoiaCdController(QString deviceDescriptor, QWidget* parent = nullptr);
        ~ParanoiaCdController();

        enum Roles {
            PathRole = Qt::UserRole,
            TitleRole,
            ArtistRole,
            AlbumRole,
            DurationRole,
            TrackRole,
            AlbumArtRole,
            ErrorRole,
            SortRole
        };

        QString albumName();
        MusicBrainzClient* musicBrainzClient();

        Q_SCRIPTABLE QCoro::Task<> eject();
        Q_SCRIPTABLE MediaItem* mediaItem(int row);

    signals:
        void albumNameChanged();
        void musicBrainzClientChanged();

    private:
        ParanoiaCdControllerPrivate* d;

        QCoro::Task<> openCd();
        void readCd();
        void feedSink();
        void setCdTextMetadata();
        void setupMusicBrainzClient();

        // QAbstractItemModel interface
    public:
        int rowCount(const QModelIndex& parent = {}) const;
        QVariant data(const QModelIndex& index, int role) const;
        QHash<int, QByteArray> roleNames() const;
};

#endif // PARANOIACDCONTROLLER_H
