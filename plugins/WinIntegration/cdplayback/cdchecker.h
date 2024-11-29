#ifndef CDCHECKER_H
#define CDCHECKER_H

#include <QAbstractListModel>
#include <QListWidgetItem>
#include <QCoroTask>
#include <winrt/CDLib.h>

class MediaItem;
struct CdCheckerPrivate;
class CdChecker : public QAbstractListModel {
        Q_OBJECT
        Q_PROPERTY(QString albumName READ albumName NOTIFY albumNameChanged FINAL)

    public:
        explicit CdChecker(QChar driveLetter, QWidget* parent = nullptr);
        ~CdChecker();

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
        CdCheckerPrivate* d;

        void checkCd();
        void getMetadata();
        void updateTrackListing();

        // QAbstractItemModel interface
    public:
        int rowCount(const QModelIndex& parent = {}) const;
        QVariant data(const QModelIndex& index, int role) const;
        QHash<int, QByteArray> roleNames() const;
};

#endif // CDCHECKER_H
