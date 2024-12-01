#ifndef PARANOIACDCONTROLLER_H
#define PARANOIACDCONTROLLER_H

#include <QAbstractListModel>
#include <QCoroTask>
#include <QWidget>

class MediaItem;
class DiskObject;
struct ParanoiaCdControllerPrivate;
class ParanoiaCdController : public QAbstractListModel {
        Q_OBJECT
        Q_PROPERTY(QString albumName READ albumName NOTIFY albumNameChanged FINAL)

    public:
        explicit ParanoiaCdController(DiskObject* disk, QWidget* parent = nullptr);
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

        Q_SCRIPTABLE QCoro::Task<> eject();
        Q_SCRIPTABLE MediaItem* mediaItem(int row);

    signals:
        void albumNameChanged();

    private:
        ParanoiaCdControllerPrivate* d;

        void readCd();
        void updateTracks();
        void feedSink();

        // QAbstractItemModel interface
    public:
        int rowCount(const QModelIndex& parent = {}) const;
        QVariant data(const QModelIndex& index, int role) const;
        QHash<int, QByteArray> roleNames() const;
};

#endif // PARANOIACDCONTROLLER_H
