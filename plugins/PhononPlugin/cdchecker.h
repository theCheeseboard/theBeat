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
#ifndef CDCHECKER_H
#define CDCHECKER_H

#include <QWidget>

namespace Ui {
    class CdChecker;
}

class QListWidgetItem;
struct CdCheckerPrivate;
class CdChecker : public QWidget {
        Q_OBJECT
    public:
        explicit CdChecker(QString blockDevice, QWidget* parent = nullptr);
        ~CdChecker();

    signals:

    private slots:
        void on_tracksWidget_itemActivated(QListWidgetItem* item);
        void checkCd();

        void on_enqueueAllButton_clicked();

        void on_ejectButton_clicked();

        void on_importCdButton_clicked();

    private:
        Ui::CdChecker* ui;
        CdCheckerPrivate* d;

        bool eventFilter(QObject* watched, QEvent* event);

        void updateTrackListing();
        void loadMusicbrainzData(QString discId);
        void selectMusicbrainzRelease(QString release);
};

#endif // CDCHECKER_H
