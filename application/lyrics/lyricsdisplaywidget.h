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
#ifndef LYRICSDISPLAYWIDGET_H
#define LYRICSDISPLAYWIDGET_H

#include <QWidget>

namespace Ui {
    class LyricsDisplayWidget;
}

class AbstractLyricFormat;
struct LyricsDisplayWidgetPrivate;
class LyricsDisplayWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit LyricsDisplayWidget(QWidget *parent = nullptr);
        ~LyricsDisplayWidget();

        void setLyrics(AbstractLyricFormat* lyrics);
        void setTime(quint64 time);

    private:
        Ui::LyricsDisplayWidget *ui;
        LyricsDisplayWidgetPrivate* d;

        void updateLyrics();

        // QWidget interface
    public:
        QSize sizeHint() const;
        QSize minimumSizeHint() const;

    protected:
        void paintEvent(QPaintEvent*event);
};

#endif // LYRICSDISPLAYWIDGET_H
