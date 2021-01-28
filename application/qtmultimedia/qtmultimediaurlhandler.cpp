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
#include "qtmultimediaurlhandler.h"

#include <QUrl>
#include "qtmultimediamediaitem.h"

QtMultimediaUrlHandler::QtMultimediaUrlHandler(QObject* parent) : UrlHandler(parent) {

}


MediaItem* QtMultimediaUrlHandler::itemForUrl(QUrl url) {
#ifndef Q_OS_MAC
    if (url.scheme() == "file" || url.scheme() == "http" || url.scheme() == "https") {
        return new QtMultimediaMediaItem(url);
    }
#endif
    return nullptr;
}
