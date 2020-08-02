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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

struct MainWindowPrivate;
class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        MainWindow(QWidget* parent = nullptr);
        ~MainWindow();

    private slots:
        void on_actionOpen_File_triggered();

        void on_actionFileBug_triggered();

        void on_actionSources_triggered();

        void on_actionAbout_triggered();

        void on_actionExit_triggered();

        void on_tracksButton_toggled(bool checked);

        void on_artistsButton_toggled(bool checked);

        void on_albumsButton_toggled(bool checked);

        void on_queueList_activated(const QModelIndex& index);

        void on_queueList_customContextMenuRequested(const QPoint& pos);

        void on_actionOpen_URL_triggered();

        void on_otherButton_toggled(bool checked);

        void on_actionSkip_Back_triggered();

        void on_actionSkip_Forward_triggered();

        void on_actionPlayPause_triggered();

        void on_playlistsButton_toggled(bool checked);

        void on_actionAdd_to_Library_triggered();

        void on_actionSettings_triggered();

    private:
        Ui::MainWindow* ui;
        MainWindowPrivate* d;

        void resizeEvent(QResizeEvent* event);
        bool eventFilter(QObject* watched, QEvent* event);

        void rewind10();
        void ff10();
};
#endif // MAINWINDOW_H
