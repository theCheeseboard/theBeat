#include "playlistmodel.h"

#include "tagcache.h"

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
        MediaItem src = sources.at(i);
        switch (role) {
            case Qt::DisplayRole: {
                QUrl url = src.source.url();
                if (src.currentType == MediaItem::Optical) {
                    return src.opticalModel->data(src.opticalModel->index(src.opticalTrack, 0));
                } else if (url.isLocalFile()) {
                    TagLib::Tag* tag = TagCache::getTag(src.source.fileName());
                    if (tag == nullptr || tag->title() == "") {
                        return QFileInfo(src.source.fileName()).baseName();
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
                } else {
                    return QIcon();
                }
            }
            case Qt::UserRole: //Return the media source
                return QVariant::fromValue(sources.at(i));
            case Qt::UserRole + 1: {
                QUrl url = src.source.url();
                if (url.isLocalFile()) {
                    TagLib::Tag* tag = TagCache::getTag(src.source.fileName());
                    if (tag == nullptr) {
                        return 0;
                    } else {
                        return tag->track();
                    }
                } else {
                    return -1;
                }
            }
            case Qt::UserRole + 3: {
                QUrl url = src.source.url();
                if (url.isLocalFile()) {
                    TagLib::AudioProperties* audio = TagCache::getAudioProperties(src.source.fileName());
                    if (audio == nullptr) {
                        return 0;
                    } else {
                        return audio->lengthInSeconds();
                    }
                } else {
                    return 0;
                }
            }
        }
        return QVariant();
    }
}

void PlaylistModel::append(MediaItem source) {
    sources.append(source);
    if (shuffle) {
        int insertInto;
        if (actualQueue.length() > 0) {
            insertInto = qrand() % actualQueue.length();
        } else {
            insertInto = 0;
        }
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
    MediaItem itemToPlay;
    if (fromActualQueue) {
        itemToPlay = actualQueue.at(i);
    } else {
        itemToPlay = sources.at(i);
    }

    if (itemToPlay.currentType == MediaItem::Optical) {
        if (mediaObj->currentSource().deviceName() == itemToPlay.source.deviceName()) {
            controller->setCurrentTitle(itemToPlay.opticalTrack + 1); //Optical Track is 0-based but the track on the CD is 1-based
        } else {
            mediaObj->setCurrentSource(itemToPlay);
            mediaObj->play();
            QMetaObject::Connection* connection = new QMetaObject::Connection;
            *connection = connect(controller, &MediaController::availableTitlesChanged, [=](int availableTitles) {
                if (availableTitles != 0) {
                    controller->setCurrentTitle(itemToPlay.opticalTrack + 1); //Optical Track is 0-based but the track on the CD is 1-based
                    disconnect(*connection);
                    delete connection;
                }
            });
        }
    } else {
        mediaObj->setCurrentSource(itemToPlay);
        mediaObj->play();
    }
}

void PlaylistModel::setRepeat(bool repeat) {
    this->repeat = repeat;
}

void PlaylistModel::setShuffle(bool shuffle) {
    this->shuffle = shuffle;

    if (currentPlayingItem == -1) return; //Queue should already be clear so there's nothing else to do right now
    MediaItem s = actualQueue.at(currentPlayingItem);
    if (shuffle) {
        actualQueue.clear();
        QList<MediaItem> notUsedSources = sources;
        while (notUsedSources.count() != 0) {
            int chosenSong = qrand() % notUsedSources.length();
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
                            int insertInto = qrand() % actualQueue.length();
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
    if (currentPlayingItem == -1 || actualQueue.length() == 0) return; //Don't do anything if there's nothing in the playlist

    if (currentPlayingItem + 1 == actualQueue.length()) {
        currentPlayingItem = -1;
    }

    currentPlayingItem++;
    playItem(currentPlayingItem, true);
}

void PlaylistModel::skipBack() {
    if (currentPlayingItem == -1 || actualQueue.length() == 0) return; //Don't do anything if there's nothing in the playlist

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

int PlaylistModel::currentItem() {
    return currentPlayingItem;
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

bool PlaylistModel::removeRow(int row, const QModelIndex &parent) {
    if (parent.isValid()) return false;
    if (row >= sources.count()) return false;

    beginRemoveRows(parent, row, row + 1);
    if (row == currentPlayingItem) {
        //Immediately skip forward if possible
        if (sources.count() == 1) {
            //Actually just clear the playlist
            clear();
            endRemoveRows();
            emit dataChanged(index(0), index(rowCount()));
            return true;
        } else {
            playNext();
            currentPlayingItem--;
        }
    } else if (currentPlayingItem != -1 && currentPlayingItem > row) {
        currentPlayingItem--;
    }

    //Remove the row
    if (shuffle) {
        //Remove any instance from the actual queue
        MediaItem item = sources.at(row);
        actualQueue.removeOne(item);
    } else {
        //Actual queue should be the same as source list
        actualQueue.removeAt(row);
    }
    sources.removeAt(row);


    endRemoveRows();
    emit dataChanged(index(0), index(rowCount()));
    return true;
}

MediaItem::MediaItem(const MediaSource &source) {
    this->source = source;
    currentType = Source;
}

MediaItem::MediaItem() {

}
