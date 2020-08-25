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
#include <QPainter>
#include <QMediaMetaData>
#include <QSet>
#include <the-libs_global.h>
#include "qtmultimedia/qtmultimediamediaitem.h"

struct PlaylistModelPrivate {
    QSet<MediaItem*> knownItems;
};

PlaylistModel::PlaylistModel(QObject* parent)
    : QAbstractListModel(parent) {
    d = new PlaylistModelPrivate();

    connect(StateManager::instance()->playlist(), &Playlist::currentItemChanged, this, [ = ] {
        emit dataChanged(index(0), index(rowCount()));
    });
    connect(StateManager::instance()->playlist(), &Playlist::itemsChanged, this, [ = ] {
        for (MediaItem* item : StateManager::instance()->playlist()->items()) {
            if (!d->knownItems.contains(item)) {
                connect(item, &MediaItem::metadataChanged, this, [ = ] {
                    emit dataChanged(index(0), index(rowCount()));
                });
                connect(item, &MediaItem::destroyed, this, [ = ] {
                    d->knownItems.remove(item);
                });
                d->knownItems.insert(item);
            }
        }
        emit dataChanged(index(0), index(rowCount()));
    });
}

PlaylistModel::~PlaylistModel() {
    delete d;
}

int PlaylistModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;

    int headers = 0;
    for (int i = 0; i < StateManager::instance()->playlist()->items().count(); i++) {
        if (drawTypeForPlaylistIndex(i) == GroupHeader) {
            headers++;
        }
    }

    return StateManager::instance()->playlist()->items().count() + headers;
}

QVariant PlaylistModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return QVariant();
    if (rowCount() <= index.row()) return QVariant();

    int priorHeaders = 0;
    for (int i = 0; i + priorHeaders < index.row(); i++) {
        if (drawTypeForPlaylistIndex(i) == GroupHeader) {
            priorHeaders++;
        }
    }

    int playlistIndex = index.row() - priorHeaders;
    MediaItem* item = StateManager::instance()->playlist()->items().at(playlistIndex);

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
        case DrawTypeRole: {
            if (index.row() != 0 && this->index(index.row() - 1).data(DrawTypeRole).value<DrawType>() == GroupHeader) {
                return GroupItem;
            }
            return drawTypeForPlaylistIndex(playlistIndex);
        }
        case PriorHeadersRole:
            return priorHeaders;
    }

    return QVariant();
}


QMimeData* PlaylistModel::mimeData(const QModelIndexList& indexes) const {
    QStringList idx;

    for (QModelIndex index : indexes) {
        idx.append(QString::number(index.row() - index.data(PriorHeadersRole).toInt()));
    }

    QMimeData* data = new QMimeData();
    data->setData("X-theBeat-PlaylistData", idx.join(",").toUtf8());
    return data;
}

bool PlaylistModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const {
    return data->hasUrls() || data->hasFormat("X-theBeat-PlaylistData");
}

bool PlaylistModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
    row -= index(row, column).data(PriorHeadersRole).toInt();
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
    flags |= Qt::ItemIsDropEnabled;
    if (index.data(DrawTypeRole).value<DrawType>() != GroupHeader) flags |= Qt::ItemIsDragEnabled;
    return flags;
}

Qt::DropActions PlaylistModel::supportedDropActions() const {
    return Qt::CopyAction;
}

bool PlaylistModel::insertRows(int row, int count, const QModelIndex& parent) {
    return true;
}

PlaylistModel::DrawType PlaylistModel::drawTypeForPlaylistIndex(int index) const {
    QList<MediaItem*> items = StateManager::instance()->playlist()->items();
    MediaItem* item = items.at(index);
    MediaItem* previousItem = nullptr;
    MediaItem* nextItem = nullptr;
    if (index != 0) {
        previousItem = items.at(index - 1);
    }
    if (index != items.count() - 1) {
        nextItem = items.at(index + 1);
    }

    if (item->album().isEmpty()) {
        return SingleItemGroup;
    } else if (previousItem && previousItem->album() == item->album()) {
        return GroupItem;
    } else if (!nextItem || nextItem->album() != item->album()) {
        return SingleItemGroup;
    } else {
        return GroupHeader;
    }
}

PlaylistDelegate::PlaylistDelegate() {

}

void PlaylistDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    MediaItem* currentItem = index.data(PlaylistModel::MediaItemRole).value<MediaItem*>();
    MediaItem* previousItem = nullptr;
    if (index.row() != 0) {
        previousItem = index.model()->index(index.row() - 1, index.column()).data(PlaylistModel::MediaItemRole).value<MediaItem*>();
    }

    QPen transientColor = option.palette.color(QPalette::Disabled, QPalette::WindowText);
    QPen textPen;

    painter->setPen(Qt::transparent);
    if (option.state & QStyle::State_Selected) {
        painter->setBrush(option.palette.brush(QPalette::Highlight));
        textPen = option.palette.color(QPalette::HighlightedText);
        transientColor = textPen;
    } else if (option.state & QStyle::State_MouseOver) {
        QColor col = option.palette.color(QPalette::Highlight);
        col.setAlpha(127);
        painter->setBrush(col);
        textPen = option.palette.color(QPalette::HighlightedText);
    } else {
        painter->setBrush(option.palette.brush(QPalette::Window));
        textPen = option.palette.color(QPalette::WindowText);
    }
    painter->drawRect(option.rect);

    PlaylistModel::DrawType drawType = index.data(PlaylistModel::DrawTypeRole).value<PlaylistModel::DrawType>();
    switch (drawType) {
        case PlaylistModel::SingleItemGroup:
        case PlaylistModel::GroupHeader: {
            QString nameText;
            QString descriptionText;

            QRect artRect = option.rect;
            artRect.setWidth(artRect.height());

            QImage artImage = currentItem->albumArt();
            if (artImage.isNull()) {
                artImage = QIcon::fromTheme("media-album-cover").pixmap(artRect.size()).toImage();
                theLibsGlobal::tintImage(artImage, option.palette.color(QPalette::WindowText));
            } else {
                artImage = artImage.scaled(artRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            }

            if (drawType == PlaylistModel::SingleItemGroup) {
                nameText = currentItem->title();
                descriptionText = currentItem->album();

                if (descriptionText.isEmpty()) descriptionText = tr("Track");

                if (StateManager::instance()->playlist()->currentItem() == currentItem) {
                    QPainter painter(&artImage);
                    painter.setBrush(option.palette.color(QPalette::Window));
                    painter.setPen(Qt::transparent);
                    painter.setOpacity(0.75);
                    painter.drawRect(0, 0, artImage.width(), artImage.height());

                    painter.setOpacity(1);

                    QRect playRect;
                    playRect.setSize(SC_DPI_T(QSize(16, 16), QSize));
                    playRect.moveCenter(QPoint(artImage.width() / 2, artImage.height() / 2));

                    QImage playImage = QIcon::fromTheme("media-playback-start").pixmap(playRect.size()).toImage();
                    theLibsGlobal::tintImage(playImage, option.palette.color(QPalette::WindowText));
                    painter.drawImage(playRect, playImage);
                }
            } else {
                nameText = currentItem->album();

                descriptionText = QLocale().createSeparatedList(currentItem->authors());
                if (descriptionText.isEmpty()) descriptionText = tr("Album");
            }

            painter->drawImage(artRect, artImage);

            QRect nameRect = option.rect;
            nameRect.setHeight(option.fontMetrics.height());
            nameRect.moveTop(option.rect.top() + 6);
            nameRect.moveLeft(artRect.right() + 6);
            nameRect.setWidth(option.fontMetrics.horizontalAdvance(nameText) + 1);

            painter->setFont(option.font);
            painter->setPen(option.palette.color(QPalette::WindowText));
            painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, nameText);

            //Draw the extra details
            QRect detailsRect = nameRect;
            detailsRect.setWidth(option.fontMetrics.horizontalAdvance(descriptionText) + 1);
            detailsRect.moveTop(nameRect.bottom());

            painter->setPen(transientColor);
            painter->drawText(detailsRect, Qt::AlignLeft | Qt::AlignVCenter, descriptionText);
            break;
        }
        case PlaylistModel::GroupItem: {
            //Draw the track number and track name
            QRect textRect = option.rect;

            textRect.setHeight(option.fontMetrics.height());
            textRect.moveTop(option.rect.top() + option.rect.height() / 2 - textRect.height() / 2);
            textRect.moveLeft(option.rect.left() + 6);

            QRect trackRect = textRect;
            trackRect.setWidth(option.fontMetrics.horizontalAdvance("99") + 1);
            textRect.setLeft(trackRect.right() + SC_DPI(6));

            painter->setPen(transientColor);

            int trackNumber = currentItem->metadata(QMediaMetaData::TrackNumber).toInt();
            if (StateManager::instance()->playlist()->currentItem() == currentItem) {
                QRect iconRect;
                iconRect.setSize(SC_DPI_T(QSize(16, 16), QSize));
                iconRect.moveCenter(trackRect.center());
                painter->drawPixmap(iconRect, QIcon::fromTheme("media-playback-start").pixmap(iconRect.size()));
            } else if (trackNumber == 0) {
                painter->drawText(trackRect, Qt::AlignCenter, "-");
            } else if (trackNumber < 99) {
                painter->drawText(trackRect, Qt::AlignCenter, QString::number(trackNumber));
            }

            painter->setPen(textPen);
            painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, currentItem->title());
            break;
        }
        QStyledItemDelegate::paint(painter, option, index);
        break;
    }

}

QSize PlaylistDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QSize sizeHint = QStyledItemDelegate::sizeHint(option, index);
    switch (index.data(PlaylistModel::DrawTypeRole).value<PlaylistModel::DrawType>()) {
        case PlaylistModel::GroupHeader:
        case PlaylistModel::SingleItemGroup:
            sizeHint.setHeight(qMin(option.fontMetrics.height() + SC_DPI(6), SC_DPI(22)) * 2);
            break;
        case PlaylistModel::GroupItem:
            sizeHint.setHeight(qMin(option.fontMetrics.height() + SC_DPI(6), SC_DPI(22)));
            break;
    }
    return sizeHint;
}
