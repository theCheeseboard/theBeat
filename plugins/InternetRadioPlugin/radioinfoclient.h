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
#ifndef RADIOINFOCLIENT_H
#define RADIOINFOCLIENT_H

#include <QObject>
#include <tpromise.h>

struct RadioInfoClientPrivate;
class RadioInfoClient : public QObject {
        Q_OBJECT
    public:
        struct Station {
            Station();
            Station(QJsonObject object, QMap<QString, QPixmap>* iconCache);

            QString stationUuid;
            QString name;
            QUrl streamUrl;
            QString icon;
            QString country;

            QMap<QString, QPixmap>* iconCache;

            QJsonObject json;
        };

        static RadioInfoClient* instance();

        static tPromise<void>* selectServer();

        static tPromise<QList<Station>>* topVoted();
        static tPromise<QList<Station>>* search(QString query);
        static tPromise<QPixmap>* getIcon(Station station);
        static void countClick(Station station);

    signals:
        void ready();

    private:
        explicit RadioInfoClient(QObject* parent = nullptr);
        RadioInfoClientPrivate* d;

};

#endif // RADIOINFOCLIENT_H
