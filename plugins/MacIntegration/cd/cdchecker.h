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
#ifndef CDCHECKER_H
#define CDCHECKER_H

#include <QAbstractListModel>
#include <QCoroTask>

class MediaItem;
class QListWidgetItem;
struct CdCheckerPrivate;
class CdChecker : public QAbstractListModel {
        Q_OBJECT
        Q_PROPERTY(QString albumName READ albumName NOTIFY albumNameChanged FINAL)
    public:
        explicit CdChecker(QString directory, QObject* parent = nullptr);
        ~CdChecker();

        enum Roles {
            PathRole = Qt::UserRole,
            TitleRole,
            ArtistRole,
            AlbumRole,
            DurationRole,
            TrackRole,
            AlbumArtRole,
            ErrorRole,
            SortRole
        };

        QString albumName();

        Q_SCRIPTABLE QCoro::Task<> eject();
        Q_SCRIPTABLE MediaItem* mediaItem(int row);

    signals:
        void albumNameChanged();
        void ejectError();

    private slots:
        QCoro::Task<> checkCd();

        void on_importCdButton_clicked();

    private:
        CdCheckerPrivate* d;

        void setupMusicBrainzClient();

        QCoro::Task<> loadMusicbrainzData(QString discId);
        QCoro::Task<> selectMusicbrainzRelease(QString release);

        // QAbstractItemModel interface
    public:
        int rowCount(const QModelIndex& parent = {}) const;
        QVariant data(const QModelIndex& index, int role) const;
        QHash<int, QByteArray> roleNames() const;
};

#endif // CDCHECKER_H
