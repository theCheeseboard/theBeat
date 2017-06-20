#include "librarymodel.h"

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
        }
    } else if (role == Qt::SizeHintRole) {
        return QSize(100, 29);
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
        return 1;
    }
}

QVariant LibraryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    } else {
        int row = index.row();
        int col = index.column();
        switch (col) {
            case 0: { //Name
                QFileInfo fileInfo(availableMediaFiles.at(row));
                if (role == Qt::DisplayRole) {
                    return fileInfo.completeBaseName();
                } else if (role == Qt::UserRole) {
                    return fileInfo.filePath();
                } else if (role == Qt::DecorationRole) {
                    return fileIconProvider.icon(fileInfo);
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
                availableMediaFiles.append(filename);
            }
        }
    }
    settings.endArray();
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
        files.append(QUrl::fromLocalFile(availableMediaFiles.at(index.row())));
    }
    mime->setUrls(files);
    return mime;
}
