/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "playlistmodel.h"

#include <statemanager.h>
#include <playlist.h>
#include <QIcon>

PlaylistModel::PlaylistModel(QObject* parent)
    : QAbstractListModel(parent) {
    connect(StateManager::instance()->playlist(), &Playlist::currentItemChanged, this, [ = ] {
        emit dataChanged(index(0), index(rowCount()));
    });
    connect(StateManager::instance()->playlist(), &Playlist::itemsChanged, this, [ = ] {
        emit dataChanged(index(0), index(rowCount()));
    });
}

int PlaylistModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;

    return StateManager::instance()->playlist()->items().count();
}

QVariant PlaylistModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return QVariant();

    MediaItem* item = StateManager::instance()->playlist()->items().at(index.row());

    switch (role) {
        case Qt::DisplayRole:
            return item->title();
        case Qt::DecorationRole:
            if (StateManager::instance()->playlist()->currentItem() == item) return QIcon::fromTheme("media-playback-start");
        case MediaItemRole:
            return QVariant::fromValue(item);
    }

    return QVariant();
}
