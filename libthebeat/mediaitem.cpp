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
#include "mediaitem.h"

#include "helpers.h"
#include <QBuffer>
#include <QImage>
#include <QUuid>
#include <QVariant>

using namespace Qt::Literals;

struct MediaItemPrivate {
        QUuid uuid = QUuid::createUuid();
};

MediaItem::MediaItem() :
    QObject(nullptr), d{new MediaItemPrivate()} {
}

MediaItem::~MediaItem() {
    delete d;
}

QUuid MediaItem::uuid() {
    return d->uuid;
}

QString MediaItem::albumArtUrl() {
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    albumArt().scaled(QSize(256, 256), Qt::KeepAspectRatio, Qt::SmoothTransformation).save(&buffer, "png");
    auto base64 = QString::fromUtf8(byteArray.toBase64());
    return QStringLiteral("data:image/png;base64,%1").arg(base64);
}

QString MediaItem::qmlAlbumArtUrl() {
    return u"image://albumart/%1"_s.arg(d->uuid.toString(QUuid::WithoutBraces));
}

int MediaItem::trackNumber() {
    return metadata(QMediaMetaData::TrackNumber).toInt();
}

QVariant MediaItem::metadata(QString key) {
    Q_UNUSED(key);
    return QVariant();
}

QVariant MediaItem::metadata(QMediaMetaData::Key key) {
    return metadata(Helpers::stringForMetadataKey(key));
}
