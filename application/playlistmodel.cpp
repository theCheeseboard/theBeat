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

#include "qtmultimedia/qtmultimediamediaitem.h"
#include <QIcon>
#include <QMediaMetaData>
#include <QMimeData>
#include <QPainter>
#include <QSet>
#include <QUrl>
#include <playlist.h>
#include <statemanager.h>
#include <the-libs_global.h>
#include <tpaintcalculator.h>

struct PlaylistModelPrivate {
        QSet<MediaItem*> knownItems;
        QVector<PlaylistModel::DrawType> drawTypes;
        QVector<int> playlistIndexes;
        QVector<int> playlistPriorHeaders;
};

PlaylistModel::PlaylistModel(QObject* parent) :
    QAbstractListModel(parent) {
    d = new PlaylistModelPrivate();

    connect(StateManager::instance()->playlist(), &Playlist::currentItemChanged, this, [=] {
        emit dataChanged(index(0), index(rowCount()));
    });
    connect(StateManager::instance()->playlist(), &Playlist::itemsChanged, this, [=] {
        for (MediaItem* item : StateManager::instance()->playlist()->items()) {
            if (!d->knownItems.contains(item)) {
                connect(item, &MediaItem::metadataChanged, this, [=] {
                    emit dataChanged(index(0), index(rowCount()));
                });
                connect(item, &MediaItem::destroyed, this, [=] {
                    d->knownItems.remove(item);
                });
                d->knownItems.insert(item);
            }
        }
        invalidateDrawTypes(0);
        emit dataChanged(index(0), index(rowCount()));
    });

    connect(this, &PlaylistModel::dataChanged, this, [=](const QModelIndex& topLeft, const QModelIndex& bottomRight) {
        invalidateDrawTypes(topLeft.row());
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

    int playlistIndex = this->playlistIndex(index.row(), &priorHeaders);
    MediaItem* item = StateManager::instance()->playlist()->items().at(playlistIndex);
    if (!item) return QVariant();

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
        case DrawTypeRole:
            return drawTypeForPlaylistIndex(index.row());
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
    if (d->drawTypes.count() > index && d->drawTypes.at(index) != NotCalculated) return d->drawTypes.at(index);

    int playlistIndex = this->playlistIndex(index);
    d->drawTypes.reserve(index + 1);

    if (index != 0 && drawTypeForPlaylistIndex(index - 1) == GroupHeader) {
        d->drawTypes.resize(index + 1);
        d->drawTypes.replace(index, GroupItem);
        return GroupItem;
    }

    QList<MediaItem*> items = StateManager::instance()->playlist()->items();
    MediaItem* item = items.at(playlistIndex);
    if (!item) {
        d->drawTypes.resize(index + 1);
        d->drawTypes.replace(index, SingleItemGroup);
        return SingleItemGroup;
    }

    MediaItem* previousItem = nullptr;
    MediaItem* nextItem = nullptr;
    if (playlistIndex != 0) {
        previousItem = items.at(playlistIndex - 1);
    }
    if (playlistIndex != items.count() - 1) {
        nextItem = items.at(playlistIndex + 1);
    }

    PlaylistModel::DrawType drawType;
    if (item->album().isEmpty()) {
        drawType = SingleItemGroup;
    } else if (previousItem && previousItem->album() == item->album()) {
        drawType = GroupItem;
    } else if (!nextItem || nextItem->album() != item->album()) {
        drawType = SingleItemGroup;
    } else {
        drawType = GroupHeader;
    }
    d->drawTypes.resize(index + 1);
    d->drawTypes.replace(index, drawType);
    return drawType;
}

void PlaylistModel::invalidateDrawTypes(int from) {
    d->drawTypes.resize(from);
    d->playlistIndexes.resize(from);
    d->playlistPriorHeaders.resize(from);
}

int PlaylistModel::playlistIndex(int index, int* priorHeaders) const {
    if (d->playlistIndexes.count() > index) {
        if (priorHeaders) *priorHeaders = d->playlistPriorHeaders.at(index);
        return d->playlistIndexes.at(index);
    }

    d->playlistIndexes.reserve(index + 1);
    d->playlistPriorHeaders.reserve(index + 1);

    int headersContainer = 0;
    if (priorHeaders == nullptr) {
        priorHeaders = &headersContainer;
    } else {
        *priorHeaders = 0;
    }

    int playlistIndex = 0;
    if (index != 0) {
        playlistIndex = this->playlistIndex(index - 1, priorHeaders) + 1;
        if (drawTypeForPlaylistIndex(index - 1) == GroupHeader) {
            (*priorHeaders)++;
            playlistIndex--;
        }
    }

    d->playlistIndexes.resize(index + 1);
    d->playlistIndexes.replace(index, playlistIndex);

    d->playlistPriorHeaders.resize(index + 1);
    d->playlistPriorHeaders.replace(index, *priorHeaders);

    return index - *priorHeaders;
}

PlaylistDelegate::PlaylistDelegate() {
}

void PlaylistDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    tPaintCalculator paintCalculator;
    paintCalculator.setPainter(painter);
    paintCalculator.setDrawBounds(option.rect);

    MediaItem* currentItem = index.data(PlaylistModel::MediaItemRole).value<MediaItem*>();

    QPen transientColor = option.palette.color(QPalette::Disabled, QPalette::WindowText);
    QPen textPen;
    QBrush brush;

    if (option.state & QStyle::State_Selected) {
        brush = option.palette.brush(QPalette::Highlight);
        textPen = option.palette.color(QPalette::HighlightedText);
        transientColor = textPen;
    } else if (option.state & QStyle::State_MouseOver) {
        QColor col = option.palette.color(QPalette::Highlight);
        col.setAlpha(127);
        brush = col;
        textPen = option.palette.color(QPalette::HighlightedText);
    } else {
        brush = option.palette.brush(QPalette::Window);
        textPen = option.palette.color(QPalette::WindowText);
    }

    paintCalculator.addRect(option.rect, [=](QRectF paintBounds) {
        painter->setPen(Qt::transparent);
        painter->setBrush(brush);
        painter->drawRect(paintBounds);
    });

    PlaylistModel::DrawType drawType = index.data(PlaylistModel::DrawTypeRole).value<PlaylistModel::DrawType>();
    switch (drawType) {
        case PlaylistModel::SingleItemGroup:
        case PlaylistModel::GroupHeader:
            {
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

                paintCalculator.addRect(artRect, [=](QRectF paintBounds) {
                    painter->drawImage(paintBounds, artImage);
                });

                QRect nameRect = option.rect;
                nameRect.setHeight(option.fontMetrics.height());
                nameRect.moveTop(option.rect.top() + 6);
                nameRect.moveLeft(artRect.right() + 6);
                nameRect.setWidth(option.fontMetrics.horizontalAdvance(nameText) + 1);

                paintCalculator.addRect(nameRect, [=](QRectF paintBounds) {
                    painter->setFont(option.font);
                    painter->setPen(option.palette.color(QPalette::WindowText));
                    painter->drawText(paintBounds, Qt::AlignLeft | Qt::AlignVCenter, nameText);
                });

                // Draw the extra details
                QRect detailsRect = nameRect;
                detailsRect.setWidth(option.fontMetrics.horizontalAdvance(descriptionText) + 1);
                detailsRect.moveTop(nameRect.bottom());

                paintCalculator.addRect(detailsRect, [=](QRectF paintBounds) {
                    painter->setPen(transientColor);
                    painter->drawText(paintBounds, Qt::AlignLeft | Qt::AlignVCenter, descriptionText);
                });
                break;
            }
        case PlaylistModel::GroupItem:
            {
                // Draw the track number and track name
                QRect textRect = option.rect;

                textRect.setHeight(option.fontMetrics.height());
                textRect.moveTop(option.rect.top() + option.rect.height() / 2 - textRect.height() / 2);
                textRect.moveLeft(option.rect.left() + 6);

                QRect trackRect = textRect;
                trackRect.setWidth(option.fontMetrics.horizontalAdvance("99") + 1);
                textRect.setLeft(trackRect.right() + SC_DPI(6));

                int trackNumber = currentItem->metadata(QMediaMetaData::TrackNumber).toInt();
                if (StateManager::instance()->playlist()->currentItem() == currentItem) {
                    QRect iconRect;
                    iconRect.setSize(SC_DPI_T(QSize(16, 16), QSize));
                    iconRect.moveCenter(trackRect.center());

                    paintCalculator.addRect(iconRect, [=](QRectF paintBounds) {
                        painter->drawPixmap(paintBounds.toRect(), QIcon::fromTheme("media-playback-start").pixmap(iconRect.size()));
                    });
                } else if (trackNumber == 0) {
                    paintCalculator.addRect(trackRect, [=](QRectF paintBounds) {
                        painter->setPen(transientColor);
                        painter->drawText(paintBounds, Qt::AlignCenter, "-");
                    });
                } else if (trackNumber < 99) {
                    paintCalculator.addRect(trackRect, [=](QRectF paintBounds) {
                        painter->setPen(transientColor);
                        painter->drawText(paintBounds, Qt::AlignCenter, QString::number(trackNumber));
                    });
                }

                paintCalculator.addRect(textRect, [=](QRectF paintBounds) {
                    painter->setPen(textPen);
                    painter->drawText(paintBounds, Qt::AlignLeft | Qt::AlignVCenter, currentItem->title());
                });
                break;
            }
        default:
            QStyledItemDelegate::paint(painter, option, index);
            break;
    }

    paintCalculator.performPaint();
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
