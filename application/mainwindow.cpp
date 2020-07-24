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
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <tcsdtools.h>
#include <QMenu>
#include <QDesktopServices>
#include <taboutdialog.h>
#include <QFileDialog>
#include <statemanager.h>
#include <playlist.h>
#include "library/librarymanager.h"
#include "playlistmodel.h"
#include <QTimer>

#include <qtmultimedia/qtmultimediamediaitem.h>

struct MainWindowPrivate {
    tCsdTools csd;

    QFrame* topBarLine;
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    d = new MainWindowPrivate();
    d->csd.installMoveAction(ui->topWidget);
    d->csd.installResizeAction(this);

    if (tCsdGlobal::windowControlsEdge() == tCsdGlobal::Left) {
        ui->leftCsdLayout->addWidget(d->csd.csdBoxForWidget(this));
    } else {
        ui->rightCsdLayout->addWidget(d->csd.csdBoxForWidget(this));
    }

    QMenu* menu = new QMenu(this);

    QMenu* helpMenu = new QMenu(this);
    helpMenu->setTitle(tr("Help"));
    helpMenu->addAction(ui->actionFileBug);
    helpMenu->addAction(ui->actionSources);
    helpMenu->addSeparator();
    helpMenu->addAction(ui->actionAbout);

    menu->addAction(ui->actionOpen_File);
    menu->addSeparator();
    menu->addMenu(helpMenu);
    menu->addAction(ui->actionExit);

    ui->menuButton->setIconSize(SC_DPI_T(QSize(24, 24), QSize));
    ui->menuButton->setMenu(menu);
    ui->stackedWidget->setCurrentAnimation(tStackedWidget::SlideHorizontal);
    ui->queueStack->setCurrentAnimation(tStackedWidget::Fade);

    connect(StateManager::instance()->playlist(), &Playlist::itemsChanged, this, [ = ] {
        ui->queueStack->setCurrentWidget(StateManager::instance()->playlist()->items().isEmpty() ? ui->queuePromptPage : ui->queueListPage);
    });
    ui->queueStack->setCurrentWidget(StateManager::instance()->playlist()->items().isEmpty() ? ui->queuePromptPage : ui->queueListPage);

    ui->artistsPage->setType(ArtistsAlbumsWidget::Artists);
    ui->albumsPage->setType(ArtistsAlbumsWidget::Albums);

    ui->queueWidget->setFixedWidth(SC_DPI(300));
    ui->queueList->setModel(new PlaylistModel);

    d->topBarLine = new QFrame(this);
    d->topBarLine->setFrameShape(QFrame::VLine);
    d->topBarLine->setFixedWidth(1);
    d->topBarLine->setParent(ui->topWidget);
    d->topBarLine->setVisible(true);
    d->topBarLine->lower();

    QTimer::singleShot(0, this, [ = ] {
        resizeEvent(nullptr);
    });
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_actionOpen_File_triggered() {
    QFileDialog* dialog = new QFileDialog(this);
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    connect(dialog, &QFileDialog::filesSelected, this, [ = ](QStringList files) {
        //TODO: Enqueue multiple items
        QtMultimediaMediaItem* item = new QtMultimediaMediaItem(QUrl::fromLocalFile(files.first()));
        StateManager::instance()->playlist()->addItem(item);
    });
    connect(dialog, &QFileDialog::finished, dialog, &QFileDialog::deleteLater);
    dialog->open();
}

void MainWindow::on_actionFileBug_triggered() {
    QDesktopServices::openUrl(QUrl("http://github.com/vicr123/theBeat/issues"));
}

void MainWindow::on_actionSources_triggered() {
    QDesktopServices::openUrl(QUrl("http://github.com/vicr123/theBeat"));
}

void MainWindow::on_actionAbout_triggered() {
    tAboutDialog d;
    d.exec();
}

void MainWindow::on_actionExit_triggered() {
    QApplication::exit();
}

void MainWindow::on_tracksButton_toggled(bool checked) {
    if (checked) {
        ui->stackedWidget->setCurrentWidget(ui->tracksPage);
    }
}

void MainWindow::on_artistsButton_toggled(bool checked) {
    if (checked) {
        ui->stackedWidget->setCurrentWidget(ui->artistsPage);
    }
}

void MainWindow::on_albumsButton_toggled(bool checked) {
    if (checked) {
        ui->stackedWidget->setCurrentWidget(ui->albumsPage);
    }
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    d->topBarLine->move(ui->queueLine->x(), 0);
    d->topBarLine->setFixedHeight(ui->topWidget->height());
}

void MainWindow::on_queueList_activated(const QModelIndex& index) {
    StateManager::instance()->playlist()->setCurrentItem(index.data(PlaylistModel::MediaItemRole).value<MediaItem*>());
}

void MainWindow::on_queueList_customContextMenuRequested(const QPoint& pos) {
    QMenu* menu = new QMenu(this);

    QModelIndexList selected = ui->queueList->selectionModel()->selectedIndexes();
    if (!selected.isEmpty()) {
        if (selected.count() == 1) {
            menu->addSection(tr("For \"%1\"").arg(menu->fontMetrics().elidedText(selected.first().data(Qt::DisplayRole).toString(), Qt::ElideMiddle, SC_DPI(300))));
        } else {
            menu->addSection(tr("For %n items", nullptr, selected.count()));
        }
        menu->addAction(QIcon::fromTheme("list-remove"), tr("Remove from Queue"), [ = ] {
            for (QModelIndex idx : selected) {
                StateManager::instance()->playlist()->removeItem(idx.data(PlaylistModel::MediaItemRole).value<MediaItem*>());
            }
        });
    }

    menu->addSection(tr("For Queue"));
    menu->addAction(QIcon::fromTheme("list-remove"), tr("Clear Queue"), [ = ] {
        StateManager::instance()->playlist()->clear();
    });

    connect(menu, &QMenu::aboutToHide, menu, &QMenu::deleteLater);
    menu->popup(ui->queueList->mapToGlobal(pos));
}
