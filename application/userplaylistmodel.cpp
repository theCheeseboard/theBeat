#include "userplaylistmodel.h"
#include "library/librarymanager.h"

UserPlaylistModel::UserPlaylistModel(QObject* parent) :
    QAbstractListModel(parent) {
    connect(LibraryManager::instance(), &LibraryManager::playlistsChanged, this, [this] {
        this->beginResetModel();
        this->endResetModel();
    });
}

int UserPlaylistModel::rowCount(const QModelIndex& parent) const {
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return LibraryManager::instance()->playlists().count() + LibraryManager::LastSmartPlaylist;
}

QVariant UserPlaylistModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return QVariant();

    auto row = index.row();
    if (row < LibraryManager::LastSmartPlaylist) {
        switch (role) {
            case Qt::DisplayRole:
                return LibraryManager::instance()->smartPlaylistName(static_cast<LibraryManager::SmartPlaylist>(row));
            case ModelRole:
                return QVariant::fromValue(LibraryManager::instance()->smartPlaylist(static_cast<LibraryManager::SmartPlaylist>(row)));
            case IDRole:
                return -1;
        }
    }

    auto playlist = LibraryManager::instance()->playlists().at(row - LibraryManager::LastSmartPlaylist);
    switch (role) {
        case Qt::DisplayRole:
            return playlist.second;
        case ModelRole:
            return QVariant::fromValue(LibraryManager::instance()->tracksByPlaylist(playlist.first));
        case IDRole:
            return playlist.first;
    }

    // FIXME: Implement me!
    return QVariant();
}

QHash<int, QByteArray> UserPlaylistModel::roleNames() const {
    return {
        {Roles::ModelRole, "model"},
        {Roles::IDRole,    "id"   },
        {Qt::DisplayRole,  "name" }
    };
}
