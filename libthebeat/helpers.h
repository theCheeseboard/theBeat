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
#ifndef HELPERS_H
#define HELPERS_H

#include "libthebeat_global.h"
#include <QObject>
#include <QCoroTask>
#include <QCache>
#include <QMediaMetaData>

class LIBTHEBEAT_EXPORT Helpers : public QObject {
        Q_OBJECT
    public:
        static QCoro::Task<QImage> albumArt(QUrl url);

        static QString stringForMetadataKey(QMediaMetaData::Key key);
        static QMediaMetaData::Key metadataKeyForString(QString string);

    private:
        static QCache<QUrl, QImage> artCache;
        static QMap<QMediaMetaData::Key, QString> metadataStrings;
};

#endif // HELPERS_H
