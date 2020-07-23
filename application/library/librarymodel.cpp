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
#include "librarymodel.h"

#include "librarymanager.h"

struct LibraryModelPrivate {

};

LibraryModel::LibraryModel(QObject* parent)
    : QSqlQueryModel(parent) {
    d = new LibraryModelPrivate();
}

LibraryModel::~LibraryModel() {
    delete d;
}

QVariant LibraryModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return QVariant();
    if (index.column() != 0) return QVariant();

    // Columns in order:
    // 0: ID
    // 1: Path
    // 2: Title
    // 3: Artist
    // 4: Album
    // 5: Duration
    // 6: Track Number

    switch (role) {
        case PathRole:
            return QSqlQueryModel::data(this->index(index.row(), 1));
        case Qt::DisplayRole:
        case TitleRole:
            return QSqlQueryModel::data(this->index(index.row(), 2));
        case ArtistRole:
            return QSqlQueryModel::data(this->index(index.row(), 3));
        case AlbumRole:
            return QSqlQueryModel::data(this->index(index.row(), 4));
        case DurationRole:
            return QSqlQueryModel::data(this->index(index.row(), 5));
        case TrackRole:
            return QSqlQueryModel::data(this->index(index.row(), 6));
    }

    return QVariant();
}
