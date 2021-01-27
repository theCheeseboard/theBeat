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
#include "scopevisualisation.h"

#include <QRect>
#include <QPainter>

struct ScopeVisualisationPrivate {

};

ScopeVisualisation::ScopeVisualisation(QObject* parent) : VisualisationEngine(parent) {
    d = new ScopeVisualisationPrivate();
}

ScopeVisualisation::~ScopeVisualisation() {
    delete d;
}

QString ScopeVisualisation::displayName() {
    return tr("Scope");
}

quint64 ScopeVisualisation::chunkSize() {
    return 2048;
}

void ScopeVisualisation::paint(QPainter* painter, QPen foreground, QRect rect) {
//    painter->fillRect(rect, QColor(255, 0, 0, 127));
    painter->setPen(foreground);
    painter->setOpacity(0.5);
    QList<qint16> chunk = this->chunk();

    if (chunk.isEmpty()) return;

    int centerY = rect.center().y();
    int previousY = (static_cast<double>(chunk.at(0)) / 32767) * (rect.height() / 2) + centerY;
    for (int i = 0; i < rect.width(); i++) {
        int point;
        if (chunk.count() <= i) {
            point = centerY;
        } else {
            point = (static_cast<double>(chunk.at(i)) / 32767) * (rect.height() / 2) + centerY;
        }
        painter->drawLine((i * 2) - 1, previousY, i * 2, point);
        previousY = point;
    }
}
