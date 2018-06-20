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

#ifndef VISUALISATIONFRAME_H
#define VISUALISATIONFRAME_H

#include <QObject>
#include <QFrame>
#include <QPainter>
#include <QtMath>
#include <complex>

class VisualisationFrame : public QFrame
{

    Q_OBJECT

public:
    VisualisationFrame(QWidget *parent);


    enum visualisationType {
        Scope,
        Bars,
        Lines,
        Circle
    };

public slots:
    void setVisualisation(QVector<qint16> visualisation);
    void setVisualisationType(visualisationType type);

signals:
    void visualisationRateChanged(int rate);

public:
    void paintEvent(QPaintEvent *paintEvent);
    void resizeEvent(QResizeEvent*);

private:
    QVector<qint16> visualisations;

    QList<qreal> previousVolumes;
    QList<std::complex<double>> ftVolumes;

    int oldRadius = -1;

    visualisationType VisType = Scope;
};

#endif // VISUALISATIONFRAME_H
