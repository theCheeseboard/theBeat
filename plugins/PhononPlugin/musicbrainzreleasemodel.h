/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2021 Victor Tran
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
#ifndef MUSICBRAINZRELEASEMODEL_H
#define MUSICBRAINZRELEASEMODEL_H

#include <QAbstractListModel>
#include <QStyledItemDelegate>
#include <musicbrainz5/ReleaseList.h>

struct MusicBrainzReleaseModelPrivate;
class MusicBrainzReleaseModel : public QAbstractListModel {
        Q_OBJECT

    public:
        explicit MusicBrainzReleaseModel(MusicBrainz5::CReleaseList releases, QObject* parent = nullptr);
        ~MusicBrainzReleaseModel();

        // Basic functionality:
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    private:
        MusicBrainzReleaseModelPrivate* d;
};

class MusicBrainzReleaseModelDelegate : public QStyledItemDelegate {
        Q_OBJECT
    public:
        explicit MusicBrainzReleaseModelDelegate(QObject* parent = nullptr);

        // QAbstractItemDelegate interface
    public:
        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // MUSICBRAINZRELEASEMODEL_H
