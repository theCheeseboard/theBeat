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
#ifndef VISUALISATIONENGINE_H
#define VISUALISATIONENGINE_H

#include "libthebeat_global.h"
#include <QObject>

class QPainter;
class VisualisationManager;
struct VisualisationEnginePrivate;
class LIBTHEBEAT_EXPORT VisualisationEngine : public QObject {
        Q_OBJECT
    public:
        explicit VisualisationEngine(QObject* parent = nullptr);
        ~VisualisationEngine();

        virtual QString displayName() = 0;
        virtual quint64 chunkSize() = 0;
        virtual void paint(QPainter* painter, QPen foreground, QRect rect) = 0;

    protected:
        friend VisualisationManager;
        void chunkAvailable(QList<qint16> chunk);
        QList<qint16> chunk();

    signals:

    private:
        VisualisationEnginePrivate* d;
};

#endif // VISUALISATIONENGINE_H
