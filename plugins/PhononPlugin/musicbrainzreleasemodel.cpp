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
#include "musicbrainzreleasemodel.h"

#include <the-libs_global.h>
#include <musicbrainz5/Release.h>
#include <musicbrainz5/ArtistCredit.h>
#include <musicbrainz5/NameCreditList.h>
#include <musicbrainz5/NameCredit.h>
#include <musicbrainz5/Artist.h>
#include <QPainter>

struct MusicBrainzReleaseModelPrivate {
    MusicBrainz5::CReleaseList releases;
};

MusicBrainzReleaseModel::MusicBrainzReleaseModel(MusicBrainz5::CReleaseList releases, QObject* parent)
    : QAbstractListModel(parent) {
    d = new MusicBrainzReleaseModelPrivate();
    d->releases = releases;
}

MusicBrainzReleaseModel::~MusicBrainzReleaseModel() {
    delete d;
}

int MusicBrainzReleaseModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;

    return d->releases.Count();
}

QVariant MusicBrainzReleaseModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return QVariant();

    MusicBrainz5::CRelease* release = d->releases.Item(index.row());
    switch (role) {
        case Qt::DisplayRole:
            return QString::fromStdString(release->Title());
        case Qt::UserRole:
            return QString::fromStdString(release->ID());
//        case Qt::UserRole + 1: {
//            QStringList artists;
//            MusicBrainz5::CNameCreditList* nameCreditList = release->ArtistCredit()->NameCreditList();
//            for (int j = 0; j < nameCreditList->NumItems(); j++) {
//                MusicBrainz5::CNameCredit* credit = nameCreditList->Item(j);
//                artists.append(QString::fromStdString(credit->Artist()->Name()));
//            }
//            artists.removeDuplicates();

//            return QLocale().createSeparatedList(artists);
//        }
        case Qt::UserRole + 1:
            return tr("Released: %1").arg(QString::fromStdString(release->Date()));
        case Qt::UserRole + 2:
            return tr("Barcode: %1").arg(QString::fromStdString(release->Barcode()));
        case Qt::UserRole + 3:
            return tr("Country: %1").arg(QString::fromStdString(release->Country()));
    }

    return QVariant();
}

MusicBrainzReleaseModelDelegate::MusicBrainzReleaseModelDelegate(QObject* parent) : QStyledItemDelegate(parent) {

}


void MusicBrainzReleaseModelDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QPen textPen;
    if (option.state & QStyle::State_MouseOver) {
        QColor col = option.palette.color(QPalette::Highlight);
        col.setAlpha(127);
        painter->setBrush(col);
        textPen = option.palette.color(QPalette::HighlightedText);
    } else {
        painter->setBrush(option.palette.color(QPalette::Window));
        textPen = option.palette.color(QPalette::WindowText);
    }
    painter->setPen(Qt::transparent);
    painter->drawRect(option.rect);
    painter->setPen(textPen);

    QFont titleFont = option.font;
    titleFont.setBold(true);

    QFontMetrics titleMetrics(titleFont);

    QRect titleRect;
    titleRect.setWidth(titleMetrics.horizontalAdvance(index.data().toString().toUpper()) + 1);
    titleRect.setHeight(titleMetrics.height());
    titleRect.moveLeft(SC_DPI(3));
    titleRect.moveTop(SC_DPI(3) + option.rect.top());
    painter->setFont(titleFont);
    painter->drawText(titleRect, index.data().toString().toUpper());

    QRect lineRect;
    lineRect.setWidth(option.rect.width() - SC_DPI(6));
    lineRect.setHeight(option.fontMetrics.height());
    lineRect.moveLeft(SC_DPI(3));
    lineRect.moveTop(titleRect.bottom());
    painter->setFont(option.font);
    painter->drawText(lineRect, index.data(Qt::UserRole + 1).toString());
    lineRect.moveTop(lineRect.bottom());
    painter->drawText(lineRect, index.data(Qt::UserRole + 2).toString());
    lineRect.moveTop(lineRect.bottom());
    painter->drawText(lineRect, index.data(Qt::UserRole + 3).toString());
}

QSize MusicBrainzReleaseModelDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QSize size;
    int maxWidth = option.fontMetrics.horizontalAdvance(index.data().toString().toUpper());
    for (int i = 0; i < 3; i++) {
        int width = option.fontMetrics.horizontalAdvance(index.data(Qt::UserRole + i).toString());
        if (width > maxWidth) maxWidth = width;
    }
    size.setWidth(maxWidth);
    size.setHeight(option.fontMetrics.height() * 4 + SC_DPI(6));
    return size;
}
