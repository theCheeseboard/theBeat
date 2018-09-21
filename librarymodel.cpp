#include "librarymodel.h"

#include <QFileSystemWatcher>

LibraryModel::LibraryModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    reloadData();
}

QVariant LibraryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return "Name";
            case 1:
                return "Artist";
            case 2:
                return "Album";
        }
    } else if (role == Qt::SizeHintRole) {
        return QSize(500, 29);
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

int LibraryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return availableMediaFiles.count();
    }
}

int LibraryModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return 3;
    }
}

QVariant LibraryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    } else {
        int row = index.row();
        int col = index.column();
        MediaFile metadata = availableMediaFiles.at(row);
        QFileInfo fileInfo(metadata.filename);
        if (role == Qt::UserRole) {
            return fileInfo.filePath();
        } else {
            switch (col) {
                case 0: { //Name
                    if (role == Qt::DisplayRole) {
                        return metadata.title;
                    } else if (role == Qt::DecorationRole) {
                        return fileIconProvider.icon(fileInfo);
                    }
                }
                case 1: { //Artist
                    if (role == Qt::DisplayRole) {
                        return metadata.artist;
                    }
                }
                case 2: { //Album
                    if (role == Qt::DisplayRole) {
                        return metadata.album;
                    }
                }
            }
        }
    }
    return QVariant();
}

void LibraryModel::reloadData() {
    availableMediaFiles.clear();

    int count = settings.beginReadArray("library/folders");
    for (int i = 0; i < count; i++) {
        settings.setArrayIndex(i);
        QDirIterator iterator(settings.value("path").toString(), QDirIterator::Subdirectories);
        while (iterator.hasNext()) {
            QString filename = iterator.next();
            if (iterator.fileInfo().isFile()) {
                MediaFile metadata;
                metadata.filename = filename;
                TagLib::FileRef tags(filename.toUtf8());
                if (tags.tag() == NULL) {
                    //Skip over this one
                    //continue;
                    metadata.title = iterator.fileInfo().completeBaseName();
                } else {
                    QString title = QString::fromStdWString(tags.tag()->title().toWString());
                    if (title != "") {
                        metadata.title = title;
                    } else {
                        metadata.title = iterator.fileInfo().completeBaseName();
                    }

                    metadata.artist = QString::fromStdWString(tags.tag()->artist().toWString());
                    metadata.album = QString::fromStdWString(tags.tag()->album().toWString());
                }

                availableMediaFiles.append(metadata);
            }
        }
    }
    settings.endArray();

    std::sort(availableMediaFiles.begin(), availableMediaFiles.end());
}

Qt::ItemFlags LibraryModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);
    return Qt::ItemIsDragEnabled | defaultFlags;
}

QStringList LibraryModel::mimeTypes() const {
    QStringList types;
    types.append("text/uri-list");
    return types;
}

QMimeData* LibraryModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData* mime = new QMimeData();
    QList<QUrl> files;
    for (QModelIndex index : indexes) {
        if (index.column() == 0) {
            files.append(QUrl::fromLocalFile(availableMediaFiles.at(index.row()).filename));
        }
    }
    mime->setUrls(files);
    return mime;
}

PlaylistFileModel::PlaylistFileModel(QObject *parent) : QAbstractListModel(parent) {
    QFileSystemWatcher* watcher = new QFileSystemWatcher();
    watcher->addPath(QDir::homePath() + "/.themedia/playlists");
    connect(watcher, &QFileSystemWatcher::directoryChanged, [=] {
        emit dataChanged(index(0), index(rowCount()));
    });
}

int PlaylistFileModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    } else {
        int numPlaylists = QDir(QDir::homePath() + "/.themedia/playlists").entryList(QDir::Files | QDir::NoDotAndDotDot).count();
        return numPlaylists;
    }
}

QVariant PlaylistFileModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    } else {
        QFileInfo fileInfo = QDir(QDir::homePath() + "/.themedia/playlists/").entryInfoList(QDir::Files | QDir::NoDotAndDotDot).at(index.row());
        if (role == Qt::DisplayRole) {
            return fileInfo.fileName();
        } else if (role == Qt::UserRole) {
            return fileInfo.filePath();
        } else {
            return QVariant();
        }
    }
}
