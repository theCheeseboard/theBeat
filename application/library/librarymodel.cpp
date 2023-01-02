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

#include "common.h"
#include "librarymanager.h"
#include <QFile>
#include <QFileInfo>
#include <QMimeData>
#include <QPainter>
#include <QUrl>
#include <helpers.h>
#include <libcontemporary_global.h>
#include <tpaintcalculator.h>

struct LibraryModelPrivate {
        TemporaryDatabase db;
};

LibraryModel::LibraryModel(QObject* parent) :
    QSqlQueryModel(parent) {
    d = new LibraryModelPrivate();
}

LibraryModel::~LibraryModel() {
    delete d;
}

QSqlDatabase LibraryModel::database() {
    return d->db.db;
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
    // 7: Sort (valid only for playlists)

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
        case AlbumArtRole:
            return QCoro::waitFor(Helpers::albumArt(QUrl::fromLocalFile(data(index, PathRole).toString())));
        case ErrorRole:
            if (!QFile::exists(data(index, PathRole).toString())) {
                return PathNotFoundError;
            }
            return NoError;
        case SortRole:
            return QSqlQueryModel::data(this->index(index.row(), 7));
    }

    return QVariant();
}

LibraryItemDelegate::LibraryItemDelegate(QObject* parent) {
}

void LibraryItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    tPaintCalculator paintCalculator;
    paintCalculator.setPainter(painter);
    paintCalculator.setDrawBounds(option.rect);

    QPen transientColor = option.palette.color(QPalette::Disabled, QPalette::WindowText);

    bool drawAsError = index.data(LibraryModel::ErrorRole).value<LibraryModel::Errors>() != LibraryModel::NoError;

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

    QRect textRect = option.rect;

    textRect.setHeight(option.fontMetrics.height());
    textRect.moveTop(option.rect.top() + 6);
    textRect.moveLeft(option.rect.left() + 6);

    // Reserve two characters' space for the track number
    QFont trackFont = option.font;
    trackFont.setPointSizeF(trackFont.pointSizeF() * 2);
    QFontMetrics trackFontMetrics(trackFont);

    QRect trackRect = textRect;
    trackRect.setWidth(trackFontMetrics.horizontalAdvance("99") + 1);
    trackRect.setHeight(trackFontMetrics.height());
    trackRect.moveTop(option.rect.top() + option.rect.height() / 2 - trackRect.height() / 2);
    textRect.setLeft(trackRect.right() + SC_DPI(6));

    // Draw the track number
    if (drawAsError) {
        paintCalculator.addRect(trackRect, [=](QRectF paintBounds) {
            painter->setFont(trackFont);
            painter->setPen(transientColor);
            painter->drawText(paintBounds, Qt::AlignCenter, "!");
        });
    } else if (index.data(LibraryModel::TrackRole).toInt() != -1) {
        int trackNumber = index.data(LibraryModel::TrackRole).toInt();
        if (trackNumber == 0) {
            paintCalculator.addRect(trackRect, [=](QRectF paintBounds) {
                painter->setFont(trackFont);
                painter->setPen(transientColor);
                painter->drawText(paintBounds, Qt::AlignCenter, "-");
            });
        } else if (trackNumber < 99) {
            paintCalculator.addRect(trackRect, [=](QRectF paintBounds) {
                painter->setFont(trackFont);
                painter->setPen(transientColor);
                painter->drawText(paintBounds, Qt::AlignCenter, QLocale().toString(trackNumber));
            });
        } else {
            // Squash the text
            qreal factor = painter->fontMetrics().horizontalAdvance("00") / static_cast<qreal>(painter->fontMetrics().horizontalAdvance(QLocale().toString(trackNumber)));
            trackRect.setWidth(trackRect.width() / factor);
            trackRect.moveLeft(trackRect.left() / factor);
            paintCalculator.addRect(trackRect, [=](QRectF paintBounds) {
                painter->save();
                painter->scale(factor, 1);
                painter->drawText(paintBounds, Qt::AlignCenter, QLocale().toString(trackNumber));
                painter->restore();
            });
        }
    }

    // Draw the track name
    QRect nameRect = textRect;
    nameRect.setWidth(option.fontMetrics.horizontalAdvance(index.data().toString()) + 1);
    textRect.setLeft(nameRect.right() + SC_DPI(6));

    paintCalculator.addRect(nameRect, [=](QRectF paintBounds) {
        painter->setFont(option.font);
        painter->setPen(drawAsError ? transientColor : option.palette.color(QPalette::WindowText));
        painter->drawText(paintBounds, Qt::AlignLeft | Qt::AlignVCenter, index.data().toString());
    });

    // Draw the extra details
    QString artist = index.data(LibraryModel::ArtistRole).toString();
    QString album = index.data(LibraryModel::AlbumRole).toString();

    QStringList details;
    if (!artist.isEmpty()) details.append(tr("by %1").arg(artist));
    if (!album.isEmpty()) details.append(tr("on %1").arg(album));
    QString detailsText = details.join(" · ");
    if (detailsText.isEmpty()) detailsText = tr("Track");

    QRect detailsRect = nameRect;
    detailsRect.setWidth(option.fontMetrics.horizontalAdvance(detailsText) + 1);
    detailsRect.moveTop(nameRect.bottom());

    paintCalculator.addRect(detailsRect, [=](QRectF paintBounds) {
        painter->setPen(transientColor);
        painter->drawText(paintBounds, Qt::AlignLeft | Qt::AlignVCenter, detailsText);
    });

    // Draw the track duration
    int duration = index.data(LibraryModel::DurationRole).toInt();
    if (duration != 0) {
        paintCalculator.addRect(textRect, [=](QRectF paintBounds) {
            painter->setPen(transientColor);
            painter->drawText(paintBounds, Qt::AlignLeft | Qt::AlignVCenter, "· " + Common::durationToString(duration));
        });
    }

    paintCalculator.performPaint();
}

QSize LibraryItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QSize sizeHint;
    sizeHint.setWidth(option.rect.width());
    sizeHint.setHeight(option.fontMetrics.height() * 2 + 12);
    return sizeHint;
}

QMimeData* LibraryModel::mimeData(const QModelIndexList& indexes) const {
    QList<QUrl> urls;
    for (QModelIndex index : indexes) {
        urls.append(QUrl::fromLocalFile(index.data(LibraryModel::PathRole).toString()));
    }

    QMimeData* data = new QMimeData();
    data->setUrls(urls);
    data->setData("X-theBeat-LibraryFlag", "true");
    return data;
}

Qt::ItemFlags LibraryModel::flags(const QModelIndex& index) const {
    Qt::ItemFlags defaultFlags = QSqlQueryModel::flags(index);
    defaultFlags |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    return defaultFlags;
}

bool LibraryModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const {
    return data->hasUrls() && !data->hasFormat("X-theBeat-LibraryFlag");
}

bool LibraryModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
    if (data->hasUrls()) {
        for (QUrl url : data->urls()) {
            if (!url.isLocalFile()) continue;
            QString file = url.toLocalFile();

            if (QFileInfo(file).isDir()) {
                LibraryManager::instance()->enumerateDirectory(file);
            } else {
                LibraryManager::instance()->addTrack(file);
            }
        }
    }
    return false;
}
