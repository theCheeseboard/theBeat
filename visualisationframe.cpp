/***************************************************************************
 *   This file is part of theMedia.
 *
 *   theMedia is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   theMedia is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with theMedia.  If not, see <http://www.gnu.org/licenses/>.
 *
****************************************************************************/

#include "visualisationframe.h"

VisualisationFrame::VisualisationFrame(QWidget* parent) : QFrame(parent)
{
    for (int i = 0; i < 1024; i++) {
        previousVolumes.append(0);
    }
}

void VisualisationFrame::setVisualisation(QVector<qint16> visualisation) {
    if (VisType == Lines || VisType == Circle) {
        qreal loudness = 0;
        qreal peak_value = 32767;

        for (int i = 0; i < visualisation.count(); i++) {
            qreal value = qAbs(qreal(visualisation.at(i)));
            if (value > loudness) {
                loudness = value;
            }
        }

        loudness /= peak_value;

        previousVolumes.append(loudness);

        while (previousVolumes.count() > this->width() / 2) {
            previousVolumes.removeFirst();
        }

        while (previousVolumes.count() < this->width() / 2) {
            previousVolumes.prepend(0);
        }
    } else {
        visualisations = visualisation;
    }
}

void VisualisationFrame::paintEvent(QPaintEvent *paintEvent) {
    QPainter painter(this);
    painter.setBrush(this->palette().brush(QPalette::WindowText));
    painter.drawLine(0, 0, this->width(), 0);

    if (VisType == Lines) {
        int i = 0;
        for (qreal level : previousVolumes) {
            //painter.drawLine(i, this->height(), i, this->height() - (level * this->height()));
            int distanceFromMiddle = (level * this->height()) / 2;
            painter.drawLine(i, this->height() / 2 + distanceFromMiddle, i, this->height() / 2 - distanceFromMiddle);
            i = i + 2;
        }
    } else if (VisType == Circle) {
        qreal level = previousVolumes.last();
        QPoint center = QPoint(this->width() / 2, this->height() / 2);
        painter.drawEllipse(center, 10, 10);

        int radius;
        if (this->height() < this->width()) {
            radius = level * this->height() / 2;
        } else {
            radius = level * this->width() / 2;
        }

        if (oldRadius != -1) {
            if (oldRadius - 10 > radius) {
                radius = oldRadius - 10;
            } else if (oldRadius + 10 < radius) {
                radius = oldRadius + 10;
            }
        }

        oldRadius = radius;

        QColor outsideColor = this->palette().color(QPalette::WindowText);

        float insideValue = ((float) radius - 50) / (float) radius;
        if (insideValue < 0) {
            insideValue = 0;
        }

        QRadialGradient outsideGrad(center, radius);
        outsideColor.setAlpha(0);
        outsideGrad.setColorAt(insideValue, outsideColor);
        outsideColor.setAlpha(40);
        outsideGrad.setColorAt(1, outsideColor);

        painter.setBrush(QBrush(outsideGrad));
        painter.setPen(this->palette().color(QPalette::WindowText));

        painter.drawEllipse(center, radius, radius);
    } else {
        if (visualisations.count() != 0) {
            qint16 oldypoint = visualisations.at(0);
            int iteration = 0;
            for (qint16 ypoint : visualisations) {
                if (VisType == Scope) {
                    painter.drawLine(iteration, oldypoint * this->height() / 2 / 32767 + this->height() / 2, iteration + 1, ypoint * this->height() / 2 / 32767 + this->height() / 2);
                    oldypoint = ypoint;
                } else if (VisType == Bars) {
                    painter.drawLine(iteration, this->height() / 2, iteration, this->height() / 2 + ypoint / 50);
                }
                iteration++;
            }
        }
    }

}

void VisualisationFrame::resizeEvent(QResizeEvent *) {
    if (this->VisType == Lines || this->VisType == Circle) {
        emit visualisationRateChanged(500);
    } else {
        emit visualisationRateChanged(this->width());
    }
}

void VisualisationFrame::setVisualisationType(visualisationType type) {
    this->VisType = type;
    if (this->VisType == Lines || this->VisType == Circle) {
        emit visualisationRateChanged(500);
    } else {
        emit visualisationRateChanged(this->width());
    }

    this->repaint();
}
