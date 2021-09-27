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
#ifndef VISUALISATIONMANAGER_H
#define VISUALISATIONMANAGER_H

#include "libthebeat_global.h"
#include <QObject>

class QPainter;
struct VisualisationManagerPrivate;
class VisualisationEngine;
class LIBTHEBEAT_EXPORT VisualisationManager : public QObject {
        Q_OBJECT
    public:
        explicit VisualisationManager(QObject* parent = nullptr);
        void setCurrentEngine(QString engine);

        QStringList engines();
        QString engineDisplayName(QString engine);

        void registerEngine(QString name, VisualisationEngine* engine);
        void provideSamples(QList<qint16> samples);

        void paint(QPainter* painter, QPen foreground, QRect rect);

    signals:
        void visualisationUpdated();

    private:
        VisualisationManagerPrivate* d;

};

#endif // VISUALISATIONMANAGER_H
