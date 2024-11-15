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
#ifndef MEDIAITEM_H
#define MEDIAITEM_H

#include "libthebeat_global.h"
#include <QMediaMetaData>
#include <QObject>
#include <QSharedPointer>

struct MediaItemPrivate;
class LIBTHEBEAT_EXPORT MediaItem : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString title READ title NOTIFY metadataChanged FINAL)
        Q_PROPERTY(QStringList authors READ authors NOTIFY metadataChanged FINAL)
        Q_PROPERTY(QString album READ album NOTIFY metadataChanged FINAL)
        Q_PROPERTY(QImage albumArt READ albumArt NOTIFY metadataChanged FINAL)
        Q_PROPERTY(QString qmlAlbumArtUrl READ qmlAlbumArtUrl NOTIFY metadataChanged FINAL)
        Q_PROPERTY(quint64 elapsed READ elapsed NOTIFY elapsedChanged FINAL)
        Q_PROPERTY(quint64 duration READ duration NOTIFY durationChanged FINAL)
        Q_PROPERTY(QString lyrics READ lyrics NOTIFY metadataChanged FINAL)
        Q_PROPERTY(QString lyricFormat READ lyricFormat NOTIFY metadataChanged FINAL)
        Q_PROPERTY(int trackNumber READ trackNumber NOTIFY metadataChanged FINAL)
    public:
        explicit MediaItem();
        ~MediaItem();

        QUuid uuid();

        Q_SCRIPTABLE virtual void play() = 0;
        Q_SCRIPTABLE virtual void pause() = 0;
        Q_SCRIPTABLE virtual void stop() = 0;
        Q_SCRIPTABLE virtual void seek(quint64 ms) = 0;
        virtual quint64 elapsed() = 0;
        virtual quint64 duration() = 0;

        virtual QString title() = 0;
        virtual QStringList authors() = 0;
        virtual QString album() = 0;
        virtual QImage albumArt() = 0;
        QString qmlAlbumArtUrl();
        int trackNumber();
        virtual QVariant metadata(QString key);

        Q_SCRIPTABLE QVariant metadata(QMediaMetaData::Key key);

        virtual QString lyrics() = 0;
        virtual QString lyricFormat() = 0;

    signals:
        void done();
        void error();
        void metadataChanged();
        void elapsedChanged();
        void durationChanged();

    private:
        MediaItemPrivate* d;
};

#endif // MEDIAITEM_H
