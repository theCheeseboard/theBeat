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
#ifndef CONTROLSTRIP_H
#define CONTROLSTRIP_H

#include <QWidget>

namespace Ui {
    class ControlStrip;
}

struct ControlStripPrivate;
class ControlStrip : public QWidget {
        Q_OBJECT

    public:
        explicit ControlStrip(QWidget* parent = nullptr);
        ~ControlStrip();

    private slots:
        void on_playPauseButton_clicked();

        void on_skipBackButton_clicked();

        void on_skipNextButton_clicked();

        void on_progressSlider_valueChanged(int value);

        void on_repeatOneButton_toggled(bool checked);

        void on_shuffleButton_toggled(bool checked);

        void on_volumeSlider_valueChanged(int value);

        void on_upButton_clicked();

        void on_repeatOneButton_customContextMenuRequested(const QPoint& pos);

    private:
        Ui::ControlStrip* ui;
        ControlStripPrivate* d;

        bool eventFilter(QObject* watched, QEvent* event);

        void updateState();
        void updateCurrentItem();
        void updateMetadata();
        void updateBar();

        void expand();
        void collapse();
};

#endif // CONTROLSTRIP_H
