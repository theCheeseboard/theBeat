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
#include "visualisationmanager.h"

#include <QMap>
#include <QRect>
#include <QPainter>
#include "visualisationengine.h"

struct VisualisationManagerPrivate {
    VisualisationEngine* currentEngine = nullptr;
    QMap<QString, VisualisationEngine*> engines;

    QList<qint16> samples;
};

VisualisationManager* VisualisationManager::instance() {
    static VisualisationManager* instance = new VisualisationManager();
    return instance;
}

QStringList VisualisationManager::engines() {
    return d->engines.keys();
}

QString VisualisationManager::engineDisplayName(QString engine) {
    return d->engines.value(engine)->displayName();
}

void VisualisationManager::setCurrentEngine(QString engine) {
    d->currentEngine = d->engines.value(engine);
}

void VisualisationManager::registerEngine(QString name, VisualisationEngine* engine) {
    d->engines.insert(name, engine);
}

void VisualisationManager::provideSamples(QList<qint16> samples) {
    if (!d->currentEngine) return;
    d->samples.append(samples);

    while (static_cast<quint64>(d->samples.length()) >= d->currentEngine->chunkSize() * 2) {
        d->samples = d->samples.mid(d->currentEngine->chunkSize());
    }

    if (static_cast<quint64>(d->samples.length()) >= d->currentEngine->chunkSize()) {
        QList<qint16> chunk = d->samples.mid(0, d->currentEngine->chunkSize());
        d->samples = d->samples.mid(d->currentEngine->chunkSize());
        d->currentEngine->chunkAvailable(chunk);

        emit visualisationUpdated();
    }
}

void VisualisationManager::paint(QPainter* painter, QPen foreground, QRect rect) {
    if (!d->currentEngine) return;

    painter->save();
    d->currentEngine->paint(painter, foreground, rect);
    painter->restore();
}

VisualisationManager::VisualisationManager(QObject* parent) : QObject(parent) {
    d = new VisualisationManagerPrivate();
}
