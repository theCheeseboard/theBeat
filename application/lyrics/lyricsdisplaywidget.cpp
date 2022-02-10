/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2022 Victor Tran
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
#include "lyricsdisplaywidget.h"
#include "ui_lyricsdisplaywidget.h"

#include <QPainter>
#include "abstractlyricformat.h"
#include <tvariantanimation.h>

struct LyricsDisplayWidgetPrivate {
    AbstractLyricFormat* lyrics = nullptr;
    quint64 time = 0;

    QStringList currentLyrics;
    quint64 currentTimePoint = 0;
    tVariantAnimation* scrollAnimation;
};

LyricsDisplayWidget::LyricsDisplayWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LyricsDisplayWidget)
{
    ui->setupUi(this);
    d = new LyricsDisplayWidgetPrivate();

    d->scrollAnimation = new tVariantAnimation(this);
    d->scrollAnimation->setStartValue(0.0);
    d->scrollAnimation->setEndValue(0.0);
    d->scrollAnimation->setCurrentTime(100);
    d->scrollAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(d->scrollAnimation, &tVariantAnimation::valueChanged, this, [=] {
        this->update();
    });
}

LyricsDisplayWidget::~LyricsDisplayWidget()
{
    delete ui;
    if (d->lyrics) d->lyrics->deleteLater();
    delete d;
}

void LyricsDisplayWidget::setLyrics(AbstractLyricFormat*lyrics)
{
    if (d->lyrics) d->lyrics->deleteLater();
    d->lyrics = lyrics;
    updateLyrics();
}

void LyricsDisplayWidget::setTime(quint64 time)
{
    d->time = time;
    updateLyrics();
}

void LyricsDisplayWidget::updateLyrics()
{
    quint64 currentTimePoint = d->lyrics->timePointForTime(d->time);
    if (currentTimePoint == d->currentTimePoint) return;

    //New lyrics!
    if (d->currentTimePoint > currentTimePoint) {
        d->scrollAnimation->setStartValue(-1.0);
    } else {
        d->scrollAnimation->setStartValue(1.0);
    }
    d->scrollAnimation->start();
    d->currentTimePoint = currentTimePoint;

    d->currentLyrics.clear();
    for (int i = -2; i <= 2; i++) {
        d->currentLyrics.append(d->lyrics->lyricsForTime(d->currentTimePoint, i));
    }

    this->update();
}

QSize LyricsDisplayWidget::sizeHint() const
{
    return QWidget::sizeHint();
}

QSize LyricsDisplayWidget::minimumSizeHint() const
{
    return QWidget::minimumSizeHint();
}

void LyricsDisplayWidget::paintEvent(QPaintEvent*event)
{
    if (!d->lyrics) return;

    QFont font = this->font();
    font.setPointSize(20);
    QFontMetrics metrics(font);

    QPainter painter(this);
    painter.setFont(font);

    QString lyric = d->lyrics->lyricsForTime(d->time);

    QRectF lyricRect;
    lyricRect.setHeight(metrics.height());
    lyricRect.setWidth(metrics.horizontalAdvance(lyric) + 1);
    lyricRect.moveCenter(QRectF(0, 0, this->width(), this->height()).center());

    QColor foregroundTransparent = this->palette().color(QPalette::WindowText);
    foregroundTransparent.setAlpha(0);

    QLinearGradient grad;
    grad.setColorAt(0, foregroundTransparent);
    grad.setColorAt(0.4, this->palette().color(QPalette::WindowText));
    grad.setColorAt(0.6, this->palette().color(QPalette::WindowText));
    grad.setColorAt(1, foregroundTransparent);

    grad.setStart(QPointF(lyricRect.left(), lyricRect.center().y() - 5 * (lyricRect.height() / 2)));
    grad.setFinalStop(QPointF(lyricRect.left(), lyricRect.center().y() + 5 * (lyricRect.height() / 2)));

    lyricRect.moveTop(lyricRect.top() + (metrics.height() * 1.75 * d->scrollAnimation->currentValue().toReal()));

    lyricRect.moveBottom(lyricRect.top() - metrics.height() * 0.75);
    lyricRect.moveBottom(lyricRect.top() - metrics.height() * 0.75);

    painter.setPen(QPen(grad, 3));
    for (QString lyric : d->currentLyrics) {
        QPointF center = lyricRect.center();
        lyricRect.setWidth(metrics.horizontalAdvance(lyric) + 1);
        lyricRect.moveCenter(center);

        painter.drawText(lyricRect, Qt::AlignCenter, lyric);
        lyricRect.moveTop(lyricRect.bottom() + metrics.height() * 0.75);
    }
}
