#ifndef LIBRARYMODEL_H
#define LIBRARYMODEL_H

#include <QAbstractTableModel>
#include <QSize>
#include <QSettings>
#include <QDirIterator>
#include <QMimeData>
#include <QUrl>
#include <QFileIconProvider>
#include <taglib/fileref.h>
#include <taglib/tag.h>

class LibraryModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit LibraryModel(QObject *parent = nullptr);

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
    void reloadData();

private:
    struct MediaFile {
        QString title;
        QString artist;
        QString filename;

        bool operator <(const MediaFile& other) const {
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

        bool operator >(const MediaFile& other) const {
            return other < *this;
        }
    };

    QList<MediaFile> availableMediaFiles;
    QSettings settings;

    QFileIconProvider fileIconProvider;
};

#endif // LIBRARYMODEL_H
