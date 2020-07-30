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
#include <QMimeData>
#include <QUrl>
#include "qtmultimedia/qtmultimediamediaitem.h"

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
    if (StateManager::instance()->playlist()->items().count() <= index.row()) return QVariant();

    MediaItem* item = StateManager::instance()->playlist()->items().at(index.row());

    switch (role) {
        case Qt::DisplayRole:
            return item->title();
        case Qt::DecorationRole:
            if (StateManager::instance()->playlist()->currentItem() == item) {
                return QIcon::fromTheme("media-playback-start");
            } else {
                return QVariant();
            }
        case MediaItemRole:
            return QVariant::fromValue(item);
    }

    return QVariant();
}


QMimeData* PlaylistModel::mimeData(const QModelIndexList& indexes) const {
    QStringList idx;

    for (QModelIndex index : indexes) {
        idx.append(QString::number(index.row()));
    }

    QMimeData* data = new QMimeData();
    data->setData("X-theBeat-PlaylistData", idx.join(",").toUtf8());
    return data;
}

bool PlaylistModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const {
    return data->hasUrls() || data->hasFormat("X-theBeat-PlaylistData");
}

bool PlaylistModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
    int insertionIndex = row < 0 ? StateManager::instance()->playlist()->items().count() : row;
    if (data->hasUrls()) {
        for (QUrl url : data->urls()) {
            StateManager::instance()->playlist()->insertItem(insertionIndex, new QtMultimediaMediaItem(url));
            insertionIndex++;
        }
    } else if (data->hasFormat("X-theBeat-PlaylistData")) {
        QStringList rows = QString(data->data("X-theBeat-PlaylistData")).split(",");
        QList<MediaItem*> itemsToInsert;
        for (QString rowStr : rows) {
            int row = rowStr.toInt();
            if (row < insertionIndex) insertionIndex--;
            itemsToInsert.append(StateManager::instance()->playlist()->takeItem(row));
        }
        for (MediaItem* item : itemsToInsert) {
            StateManager::instance()->playlist()->insertItem(insertionIndex, item);
            insertionIndex++;
        }
    }
    return false;
}

Qt::ItemFlags PlaylistModel::flags(const QModelIndex& index) const {
    Qt::ItemFlags flags = QAbstractListModel::flags(index);
    flags |= Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled;
    return flags;
}

Qt::DropActions PlaylistModel::supportedDropActions() const {
    return Qt::CopyAction;
}

bool PlaylistModel::insertRows(int row, int count, const QModelIndex& parent) {
    return true;
}
