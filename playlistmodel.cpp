#include "playlistmodel.h"

PlaylistModel::PlaylistModel(MediaObject* object, QObject *parent)
    : QAbstractListModel(parent)
{
    this->mediaObj = object;

    connect(object, SIGNAL(currentSourceChanged(Phonon::MediaSource)), this, SLOT(mediaChanged(Phonon::MediaSource)));

    controller = new MediaController(object);
}

int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid()) {
        return 0;
    } else {
        return sources.count();
    }
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    } else {
        int i = index.row();
        switch (role) {
            case Qt::DisplayRole: {
                MediaSource src = sources.at(i);
                QUrl url = src.url();
                if (url.isLocalFile()) {
                    //TagLib::FileRef tags(src.fileName().toUtf8());
                    //if (tags.tag() == NULL) {
                        return QFileInfo(src.fileName()).baseName();
                    //} else {
                        //return QString::fromStdWString(tags.tag()->title().toWString());
                    //}
                } else {
                    return url.toDisplayString();
                }
            }
            case Qt::DecorationRole:
                if (i == currentPlayingItem) {
                    return QIcon::fromTheme("media-playback-start");
                } else {
                    return fileIconProvider.icon(QFileInfo(sources.at(i).source.fileName()));
                }
            case Qt::UserRole: //Return the media source
                return QVariant::fromValue(sources.at(i));
        }
        return QVariant();
    }
}

void PlaylistModel::append(MediaSource source) {
    sources.append(source);
    dataChanged(this->index(0), this->index(rowCount()));
}

void PlaylistModel::appendPlaylist(QString path) {
    QFile open(path);
    open.open(QFile::ReadOnly);
    QString contents(open.readAll());
    open.close();

    for (QString file : contents.split("\n")) {
        if (file != "") {
            MediaSource source(file);
            sources.append(source);
        }
    }
    dataChanged(this->index(0), this->index(rowCount()));
}

void PlaylistModel::enqueueNext() {
    if (repeat && currentPlayingItem != -1) {
        mediaObj->enqueue(sources.at(currentPlayingItem));
    } else {
        if (currentPlayingItem + 1 == rowCount()) {
            currentPlayingItem = -1;
        }

        currentPlayingItem++;
        mediaObj->enqueue(sources.at(currentPlayingItem));
    }
}

void PlaylistModel::enqueueAndPlayNext() {
    enqueueNext();
    if (mediaObj->state() != PlayingState) {
        mediaObj->play();
    }
}

void PlaylistModel::mediaChanged(MediaSource source) {
    if (sources.contains(source)) {
        currentPlayingItem = sources.indexOf(source);
    }
    dataChanged(this->index(0), this->index(rowCount()));
}

void PlaylistModel::playItem(int i) {
    mediaObj->setCurrentSource(sources.at(i));
    mediaObj->play();
}

void PlaylistModel::setRepeat(bool repeat) {
    this->repeat = repeat;
}

bool PlaylistModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) {
    if (action == Qt::CopyAction) {
        if (data->hasUrls()) {
            bool append = !parent.isValid();

            for (QUrl url : data->urls()) {
                if (url.toLocalFile().startsWith(QDir::homePath() + "/.themedia/playlists/")) {
                    QFile open(url.toLocalFile());
                    open.open(QFile::ReadOnly);
                    QString contents(open.readAll());
                    open.close();

                    for (QString file : contents.split("\n")) {
                        if (file != "") {
                            MediaSource source(file);
                            sources.append(source);
                        }
                    }
                } else {
                    MediaSource source(url);
                    if (append) {
                        sources.append(source);
                    } else {
                        sources.insert(parent.row(), source);
                    }
                }
            }
            dataChanged(this->index(0), this->index(rowCount()));
            return true;
        } else {
            return false;
        }
    } else if (action == Qt::IgnoreAction) {
        return true;
    }
    return true;
}

Qt::DropActions PlaylistModel::supportedDropActions() const {
    return Qt::CopyAction;
}

bool PlaylistModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) {
    //QList<MediaSource> sourcesToMove = sources.mid(sourceParent.row(), count);
    for (int i = 0; i < count; i++) {
        sources.move(sourceParent.row(), destinationParent.row());
    }
    return true;
}

Qt::ItemFlags PlaylistModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    return Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled | defaultFlags;
}

QStringList PlaylistModel::mimeTypes() const {
    QStringList types;
    types.append("text/uri-list");
    return types;
}

QMimeData* PlaylistModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData* mime = new QMimeData();
    QList<QUrl> files;
    for (QModelIndex index : indexes) {
        files.append(sources.at(index.row()).source.url());
    }
    mime->setUrls(files);
    return mime;
}

void PlaylistModel::playNext() {
    if (currentPlayingItem + 1 == rowCount()) {
        currentPlayingItem = -1;
    }

    currentPlayingItem++;
    playItem(currentPlayingItem);
}

void PlaylistModel::skipBack() {
    if (mediaObj->currentTime() < 5000) {
        if (currentPlayingItem - 1 == -1) {
            currentPlayingItem = rowCount();
        }

        currentPlayingItem--;
        playItem(currentPlayingItem);
    } else {
        mediaObj->seek(0);
    }
}

MediaItem::MediaItem(const MediaSource &source) {
    this->source = source;
    currentType = Source;
}

MediaItem::MediaItem() {

}
