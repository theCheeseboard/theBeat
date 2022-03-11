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
#ifndef TRACKINFO_H
#define TRACKINFO_H

#include <QObject>

struct TrackInfoPrivate;
class TrackInfo : public QObject {
        Q_OBJECT
    public:
        explicit TrackInfo();
        explicit TrackInfo(int track);
        ~TrackInfo();

        QString title();
        QStringList artist();
        QString album();
        int track();
        QImage albumArt();

        void setData(QString title, QStringList artist, QString album);
        void setData(TrackInfo* trackInfo);
        void setAlbumArt(QImage albumArt);

    signals:
        void dataChanged();

    private:
        TrackInfoPrivate* d;
};

typedef QSharedPointer<TrackInfo> TrackInfoPtr;

#endif // TRACKINFO_H
