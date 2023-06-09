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
#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QAbstractListModel>
#include <QStyledItemDelegate>

struct PlaylistModelPrivate;
class PlaylistModel : public QAbstractListModel {
        Q_OBJECT

    public:
        explicit PlaylistModel(QObject* parent = nullptr);
        ~PlaylistModel();

        enum Roles {
            MediaItemRole = Qt::UserRole,
            DrawTypeRole,
            PriorHeadersRole
        };

        enum DrawType {
            NotCalculated = 0,
            GroupHeader,
            GroupItem,
            SingleItemGroup
        };

        // Basic functionality:
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;

        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        QMimeData* mimeData(const QModelIndexList& indexes) const override;
        bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const override;
        bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
        Qt::ItemFlags flags(const QModelIndex& index) const override;
        Qt::DropActions supportedDropActions() const override;
        bool insertRows(int row, int count, const QModelIndex& parent) override;

        DrawType drawTypeForPlaylistIndex(int index) const;
        void invalidateDrawTypes(int from);

        int playlistIndex(int index, int* priorHeaders = nullptr) const;

    private:
        PlaylistModelPrivate* d;
};
Q_DECLARE_METATYPE(PlaylistModel::DrawType);

class PlaylistDelegate : public QStyledItemDelegate {
        Q_OBJECT

    public:
        PlaylistDelegate();

        // QAbstractItemDelegate interface
    public:
        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // PLAYLISTMODEL_H
