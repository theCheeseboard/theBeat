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
#ifndef LIBRARYMODEL_H
#define LIBRARYMODEL_H

#include <QSqlQueryModel>
#include <QStyledItemDelegate>

struct LibraryModelPrivate;
class LibraryModel : public QSqlQueryModel {
        Q_OBJECT

    public:
        explicit LibraryModel(QObject* parent = nullptr);
        ~LibraryModel();

        enum Roles {
            PathRole = Qt::UserRole,
            TitleRole,
            ArtistRole,
            AlbumRole,
            DurationRole,
            TrackRole,
            AlbumArtRole,
            ErrorRole
        };

        enum Errors {
            NoError,
            PathNotFoundError
        };

        // Basic functionality:
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        QMimeData* mimeData(const QModelIndexList& indexes) const override;
        Qt::ItemFlags flags(const QModelIndex& index) const override;
        bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const override;
        bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

    private:
        LibraryModelPrivate* d;

};
Q_DECLARE_METATYPE(LibraryModel::Errors)

class LibraryItemDelegate : public QStyledItemDelegate {
        Q_OBJECT

    public:
        explicit LibraryItemDelegate(QObject* parent = nullptr);

        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

#endif // LIBRARYMODEL_H
