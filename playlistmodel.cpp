#include "playlistmodel.h"

PlaylistModel::PlaylistModel(MediaObject* object, QObject *parent)
    : QAbstractListModel(parent)
{
    this->mediaObj = object;

    connect(object, SIGNAL(currentSourceChanged(Phonon::MediaSource)), this, SLOT(mediaChanged(Phonon::MediaSource)));
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
            case Qt::DisplayRole:
                return QFileInfo(sources.at(i).fileName()).baseName();
            case Qt::DecorationRole:
                if (i == currentPlayingItem) {
                    return QIcon::fromTheme("media-playback-start");
                } else {
                    return fileIconProvider.icon(QFileInfo(sources.at(i).fileName()));
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

void PlaylistModel::enqueueNext() {
    if (currentPlayingItem + 1 == rowCount()) {
        currentPlayingItem = -1;
    }

    currentPlayingItem++;
    mediaObj->enqueue(sources.at(currentPlayingItem));
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
