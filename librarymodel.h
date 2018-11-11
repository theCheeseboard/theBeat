#ifndef LIBRARYMODEL_H
#define LIBRARYMODEL_H

#include <QAbstractTableModel>
#include <QSize>
#include <QSettings>
#include <QDirIterator>
#include <QMimeData>
#include <QUrl>
#include <QMimeDatabase>
#include <QStyledItemDelegate>
#include "tagcache.h"

class LibraryModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit LibraryModel(QObject *parent = nullptr);

    enum QueryType {
        None = -1,
        Track = 0,
        Artist = 1,
        Album = 2
    };

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList &indexes) const override;

public slots:
    int reloadData();
    void search(QString query);
    void filter(QString match, QueryType filter);
    void clearFilter();

private:
    struct MediaFile {
        static bool compareByTrackNumber;
        QString title;
        QString artist;
        QString filename;
        QString album;
        uint trackNumber;
        int duration;

        bool operator <(const MediaFile& other) const {
            if (compareByTrackNumber) {
                return trackNumber < other.trackNumber;
            } else {
                QString compare1, compare2;
                if (title != "" && other.title != "") {
                    compare1 = title;
                    compare2 = other.title;
                } else if (title == "" && other.title != "") {
                    QFileInfo fileInfo1(filename);
                    compare1 = fileInfo1.completeBaseName();
                    compare2 = other.title;
                } else if (title != "" && other.title == "") {
                    QFileInfo fileInfo2(other.filename);
                    compare1 = title;
                    compare2 = fileInfo2.completeBaseName();
                } else {
                    QFileInfo fileInfo1(filename);
                    QFileInfo fileInfo2(other.filename);
                    compare1 = fileInfo1.completeBaseName();
                    compare2 = fileInfo2.completeBaseName();
                }

                int compare = compare1.localeAwareCompare(compare2);
                if (compare < 0) {
                    return true;
                } else {
                    return false;
                }
            }
        }

        bool operator >(const MediaFile& other) const {
            return other < *this;
        }
    };

    QList<MediaFile> availableMediaFiles, shownMediaFiles;
    QSettings settings;

    QString currentSearchQuery;
    QString currentFilter;
    QueryType currentFilterType = None;

    QMimeDatabase mimedb;
};

class LibraryTitleDelegate : public QStyledItemDelegate {
    Q_OBJECT

    public:
        explicit LibraryTitleDelegate(QObject* parent = nullptr);

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class PlaylistFileModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit PlaylistFileModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

class ArtistLibraryModel : public QAbstractListModel {
    Q_OBJECT

    public:
        explicit ArtistLibraryModel(LibraryModel* libraryModel, QObject* parent = nullptr);

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    public slots:
        void updateData();

    private:
        LibraryModel* libraryModel;
        QStringList artists;
};

class AlbumLibraryModel : public QAbstractListModel {
    Q_OBJECT

    public:
        explicit AlbumLibraryModel(LibraryModel* libraryModel, QObject* parent = nullptr);

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    public slots:
        void updateData();

    private:
        LibraryModel* libraryModel;
        QStringList albums;
};

#endif // LIBRARYMODEL_H
