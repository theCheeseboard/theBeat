#ifndef GSTCDCONTROLLER_H
#define GSTCDCONTROLLER_H

#include <QAbstractListModel>
#include <QCoroTask>
#include <QWidget>

class MediaItem;
class DiskObject;
struct GstCdControllerPrivate;
class GstCdController : public QAbstractListModel {
        Q_OBJECT
        Q_PROPERTY(QString albumName READ albumName NOTIFY albumNameChanged FINAL)

    public:
        explicit GstCdController(DiskObject* disk, QWidget* parent = nullptr);
        ~GstCdController();

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
        GstCdControllerPrivate* d;

        void readCd();
        void updateTracks();

        // QAbstractItemModel interface
    public:
        int rowCount(const QModelIndex& parent = {}) const;
        QVariant data(const QModelIndex& index, int role) const;
        QHash<int, QByteArray> roleNames() const;
};

#endif // GSTCDCONTROLLER_H
