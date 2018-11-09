#include "playlistmodel.h"

#include "tagcache.h"
#include <QRandomGenerator>

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
                    TagLib::Tag* tag = TagCache::getTag(src.fileName());
                    if (tag == nullptr) {
                        return QFileInfo(src.fileName()).baseName();
                    } else {
                        return QString::fromStdWString(tag->title().toWString());
                    }
                } else {
                    return url.toDisplayString();
                }
            }
            case Qt::DecorationRole: {
                MediaSource current = sources.at(i).source;

                if (currentPlayingItem != -1 && i == sources.indexOf(actualQueue.at(currentPlayingItem))) {
                    return QIcon::fromTheme("media-playback-start");
                } else if (current.type() == MediaSource::LocalFile) {
                    QMimeType t = mimeDb.mimeTypeForFile(current.fileName());
                    return QIcon::fromTheme(t.iconName());
                } else if (current.type() == MediaSource::Url) {
                    QUrl u = current.url();
                    if (u.isLocalFile()) {
                        QMimeType t = mimeDb.mimeTypeForUrl(current.url());
                        return QIcon::fromTheme(t.iconName());
                    } else {
                        return QIcon::fromTheme("network-wireless-connected-75");
                    }
                }
            }
            case Qt::UserRole: //Return the media source
                return QVariant::fromValue(sources.at(i));
        }
        return QVariant();
    }
}

void PlaylistModel::append(MediaSource source) {
    sources.append(source);
    if (shuffle) {
        int insertInto = QRandomGenerator::system()->bounded(actualQueue.length());
        actualQueue.insert(insertInto, source);

        if (insertInto <= currentPlayingItem && currentPlayingItem != 0) {
            currentPlayingItem++;
        }
    } else {
        actualQueue.append(source);
    }
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
            append(source);
        }
    }
    //dataChanged(this->index(0), this->index(rowCount()));
}

void PlaylistModel::enqueueNext() {
    if (repeat && currentPlayingItem != -1) {
        mediaObj->enqueue(actualQueue.at(currentPlayingItem));
    } else {
        if (currentPlayingItem + 1 == rowCount()) {
            currentPlayingItem = -1;
        }

        currentPlayingItem++;
        mediaObj->enqueue(actualQueue.at(currentPlayingItem));
    }
}

void PlaylistModel::enqueueAndPlayNext() {
    enqueueNext();
    if (mediaObj->state() != PlayingState) {
        mediaObj->play();
    }
}

void PlaylistModel::mediaChanged(MediaSource source) {
    if (actualQueue.contains(source)) {
        currentPlayingItem = actualQueue.indexOf(source);
    }
    dataChanged(this->index(0), this->index(rowCount()));
}

void PlaylistModel::playItem(int i, bool fromActualQueue) {
    if (fromActualQueue) {
        mediaObj->setCurrentSource(actualQueue.at(i));
    } else {
        mediaObj->setCurrentSource(sources.at(i));
    }
    mediaObj->play();
}

void PlaylistModel::setRepeat(bool repeat) {
    this->repeat = repeat;
}

void PlaylistModel::setShuffle(bool shuffle) {
    this->shuffle = shuffle;
    MediaItem s = actualQueue.at(currentPlayingItem);
    if (shuffle) {
        actualQueue.clear();
        QList<MediaItem> notUsedSources = sources;
        while (notUsedSources.count() != 0) {
            int chosenSong = QRandomGenerator::system()->bounded(notUsedSources.length());
            actualQueue.append(notUsedSources.at(chosenSong));
            notUsedSources.removeAt(chosenSong);
        }
    } else {
        //Return to playing sequentially
        actualQueue = sources;
    }

    for (int i = 0; i < actualQueue.length(); i++) {
        if (actualQueue.at(i).source == s) {
            currentPlayingItem = i;
            break;
        }
    }
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
                            this->append(source);
                        }
                    }
                } else {
                    MediaSource source(url);
                    if (append) {
                        this->append(source);
                    } else {
                        sources.insert(parent.row(), source);

                        if (shuffle) {
                            int insertInto = QRandomGenerator::system()->bounded(actualQueue.length());
                            actualQueue.insert(insertInto, source);

                            if (insertInto <= currentPlayingItem && currentPlayingItem != 0) {
                                currentPlayingItem++;
                            }
                        } else {
                            actualQueue.append(source);
                        }
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
    //if (shuffle) {
        //playItem(QRandomGenerator::system()->bounded(sources.length()));
    //} else {
        if (currentPlayingItem + 1 == actualQueue.length()) {
            currentPlayingItem = -1;
        }

        currentPlayingItem++;
        playItem(currentPlayingItem, true);
    //}
}

void PlaylistModel::skipBack() {
    if (mediaObj->currentTime() < 5000) {
        if (currentPlayingItem - 1 == -1) {
            currentPlayingItem = actualQueue.length();
        }

        currentPlayingItem--;
        playItem(currentPlayingItem, true);
    } else {
        mediaObj->seek(0);
    }
}

void PlaylistModel::clear() {
    currentPlayingItem = -1;
    sources.clear();
    actualQueue.clear();
    mediaObj->stop();
    dataChanged(this->index(0), this->index(rowCount()));
}

QByteArray PlaylistModel::createPlaylist() {
    QStringList playlistEntries;
    for (MediaSource src : sources) {
        if (src.type() == MediaSource::LocalFile) {
            playlistEntries.append(src.fileName());
        } else if (src.type() == MediaSource::Url) {
            playlistEntries.append(src.url().toString());
        }
    }
    return playlistEntries.join("\n").toUtf8();
}

MediaItem::MediaItem(const MediaSource &source) {
    this->source = source;
    currentType = Source;
}

MediaItem::MediaItem() {

}
