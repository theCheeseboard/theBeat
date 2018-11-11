#include "librarymodel.h"

#include <QFileSystemWatcher>
#include <QIcon>
#include <QPainter>
#include <QTime>
#include <the-libs_global.h>

bool LibraryModel::MediaFile::compareByTrackNumber = false;

LibraryModel::LibraryModel(QObject *parent)
    : QAbstractTableModel(parent)
{

}

QVariant LibraryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return tr("Name", "Name of a music track");
            case 1:
                return tr("Artist");
            case 2:
                return tr("Album");
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
        return shownMediaFiles.count();
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

        if (shownMediaFiles.length() <= row) {
            //Stop
            return QVariant();
        }
        MediaFile metadata = shownMediaFiles.at(row);
        QFileInfo fileInfo(metadata.filename);
        if (role == Qt::UserRole) {
            return fileInfo.filePath();
        } else {
            switch (col) {
                case Track: {
                    if (role == Qt::DisplayRole) {
                        return metadata.title;
                    } else if (role == Qt::DecorationRole) {
                        QMimeType t = mimedb.mimeTypeForFile(fileInfo);
                        return QIcon::fromTheme(t.iconName());
                    } else if (role == Qt::UserRole + 1) { //Track Number
                        return metadata.trackNumber;
                    } else if (role == Qt::UserRole + 2) { //Duration
                        return metadata.duration;
                    }
                }
                case Artist: {
                    if (role == Qt::DisplayRole) {
                        return metadata.artist;
                    }
                }
                case Album: {
                    if (role == Qt::DisplayRole) {
                        return metadata.album;
                    }
                }
            }
        }
    }
    return QVariant();
}

int LibraryModel::reloadData() {
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

                TagLib::Tag* tag = TagCache::getTag(filename);
                if (tag == nullptr) {
                    //Skip over this one
                    //continue;
                    metadata.title = iterator.fileInfo().completeBaseName();
                } else {
                    TagLib::AudioProperties* audio = TagCache::getAudioProperties(filename);
                    QString title = QString::fromStdWString(tag->title().toWString());
                    if (title != "") {
                        metadata.title = title;
                    } else {
                        metadata.title = iterator.fileInfo().completeBaseName();
                    }

                    metadata.artist = QString::fromStdWString(tag->artist().toWString());
                    metadata.album = QString::fromStdWString(tag->album().toWString());
                    metadata.trackNumber = tag->track();
                    metadata.duration = audio->lengthInMilliseconds();
                }

                availableMediaFiles.append(metadata);
            }
        }
    }
    settings.endArray();

    std::sort(availableMediaFiles.begin(), availableMediaFiles.end());

    shownMediaFiles = availableMediaFiles;

    if (availableMediaFiles.count() == 0) {
        return 1;
    } else {
        return 0;
    }
}

void LibraryModel::search(QString query) {
    currentSearchQuery = query;
    QList<MediaFile> intermediateMediaFiles;

    if (query == "") {
        intermediateMediaFiles = availableMediaFiles;
    } else {
        for (MediaFile file : availableMediaFiles) {
            if (file.title.contains(query, Qt::CaseInsensitive) || file.artist.contains(query, Qt::CaseInsensitive) || file.album.contains(query, Qt::CaseInsensitive)) {
                intermediateMediaFiles.append(file);
            }
        }
    }

    if (currentFilterType == None) {
        shownMediaFiles = intermediateMediaFiles;
    } else {
        shownMediaFiles.clear();
        for (MediaFile file : availableMediaFiles) {
            switch (currentFilterType) {
                case Artist:
                    if (file.artist == currentFilter) shownMediaFiles.append(file);
                    break;
                case Track:
                    if (file.title == currentFilter) shownMediaFiles.append(file);
                    break;
                case Album:
                    if (file.album == currentFilter) shownMediaFiles.append(file);
                    break;
            }
        }
    }

    if (currentFilterType == Album) {
        //Sort by track number
        MediaFile::compareByTrackNumber = true;
        std::sort(shownMediaFiles.begin(), shownMediaFiles.end());
        MediaFile::compareByTrackNumber = false;
    }

    emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
    emit layoutChanged();
}

void LibraryModel::filter(QString filter, QueryType filterType) {
    this->currentFilter = filter;
    this->currentFilterType = filterType;

    search(currentSearchQuery);
}

void LibraryModel::clearFilter() {
    currentFilter.clear();
    currentFilterType = None;

    search(currentSearchQuery);
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
            files.append(QUrl::fromLocalFile(shownMediaFiles.at(index.row()).filename));
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

ArtistLibraryModel::ArtistLibraryModel(LibraryModel* libraryModel, QObject* parent) : QAbstractListModel(parent) {
    this->libraryModel = libraryModel;

    connect(libraryModel, &LibraryModel::dataChanged, this, &ArtistLibraryModel::updateData);
    updateData();
}

int ArtistLibraryModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    } else {
        return artists.count();
    }
}

void ArtistLibraryModel::updateData() {
    artists.clear();

    for (int i = 0; i < libraryModel->rowCount(); i++) {
        QString artist = libraryModel->data(libraryModel->index(i, LibraryModel::Artist)).toString();
        if (!artists.contains(artist) && artist != "") {
            artists.append(artist);
        }
    }

    std::sort(artists.begin(), artists.end());
    emit dataChanged(index(0), index(rowCount()));
}

QVariant ArtistLibraryModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    } else {
        int row = index.row();

        if (artists.length() <= row) {
            //Stop
            return QVariant();
        }

        if (role == Qt::DisplayRole) {
            return artists.at(row);
        }
    }
    return QVariant();
}


AlbumLibraryModel::AlbumLibraryModel(LibraryModel* libraryModel, QObject* parent) : QAbstractListModel(parent) {
    this->libraryModel = libraryModel;

    connect(libraryModel, &LibraryModel::dataChanged, this, &AlbumLibraryModel::updateData);
    updateData();
}

int AlbumLibraryModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    } else {
        return albums.count();
    }
}

void AlbumLibraryModel::updateData() {
    albums.clear();

    for (int i = 0; i < libraryModel->rowCount(); i++) {
        QString album = libraryModel->data(libraryModel->index(i, LibraryModel::Album)).toString();
        if (!albums.contains(album) && album != "") {
            albums.append(album);
        }
    }

    std::sort(albums.begin(), albums.end());
    emit dataChanged(index(0), index(rowCount()));
}

QVariant AlbumLibraryModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    } else {
        int row = index.row();

        if (albums.length() <= row) {
            //Stop
            return QVariant();
        }

        if (role == Qt::DisplayRole) {
            return albums.at(row);
        }
    }
    return QVariant();
}

LibraryTitleDelegate::LibraryTitleDelegate(QObject* parent) : QStyledItemDelegate(parent) {

}


void LibraryTitleDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->setPen(Qt::transparent);
    QPen textPen;
    if (option.state & QStyle::State_Selected) {
        painter->setBrush(option.palette.brush(QPalette::Highlight));
        textPen = option.palette.color(QPalette::HighlightedText);
    } else if (option.state & QStyle::State_MouseOver) {
        QColor col = option.palette.color(QPalette::Highlight);
        col.setAlpha(127);
        painter->setBrush(col);
        textPen = option.palette.color(QPalette::HighlightedText);
    } else {
        painter->setBrush(option.palette.brush(QPalette::Window));
        textPen = option.palette.color(QPalette::WindowText);
    }
    painter->drawRect(option.rect);

    QRect iconRect, textRect = option.rect;

    iconRect.setSize(QSize(16, 16) * theLibsGlobal::getDPIScaling());
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    QImage iconImage = icon.pixmap(iconRect.size()).toImage();
    iconRect.moveLeft(option.rect.left() + 2 * theLibsGlobal::getDPIScaling());
    iconRect.moveTop(option.rect.top() + (option.rect.height() / 2) - (iconRect.height() / 2));
    painter->drawImage(iconRect, iconImage);
    textRect.setLeft(iconRect.right() + 6 * theLibsGlobal::getDPIScaling());

    //Reserve two characters' space for the track number
    QRect trackRect = textRect;
    trackRect.setWidth(option.fontMetrics.width("99") + 1);
    textRect.setLeft(trackRect.right() + 6 * theLibsGlobal::getDPIScaling());

    //Draw the track number
    painter->setFont(option.font);
    painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
    if (index.data(Qt::UserRole + 1) == 0) {
        painter->drawText(trackRect, Qt::AlignRight | Qt::AlignVCenter, "-");
    } else {
        painter->drawText(trackRect, Qt::AlignRight | Qt::AlignVCenter, QString::number(index.data(Qt::UserRole + 1).toInt()));
    }

    //Draw the track name
    QRect nameRect = textRect;
    nameRect.setWidth(option.fontMetrics.width(index.data().toString()) + 1);
    textRect.setLeft(nameRect.right() + 6 * theLibsGlobal::getDPIScaling());

    painter->setPen(option.palette.color(QPalette::WindowText));
    painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, index.data().toString());

    //Draw the track duration
    painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
    QTime duration = QTime::fromMSecsSinceStartOfDay(index.data(Qt::UserRole + 2).toInt());
    QString durationString;
    if (duration.hour() == 0) {
        durationString = duration.toString("mm:ss");
    } else {
        durationString = duration.toString("hh:mm:ss");
    }
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, "- " + durationString);
}
