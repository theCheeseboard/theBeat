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
#include <QShortcut>
#include "pluginmanager.h"

#include <qtmultimedia/qtmultimediamediaitem.h>

#ifdef Q_OS_WIN
    #include <QWinThumbnailToolBar>
#include <QWinThumbnailToolButton>
    #include "platformintegration/winplatformintegration.h"
#endif

struct MainWindowPrivate {
    tCsdTools csd;
    PluginManager plugins;

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
    menu->addAction(ui->actionOpen_URL);
    menu->addSeparator();
    menu->addAction(ui->actionPlayPause);
    menu->addAction(ui->actionSkip_Back);
    menu->addAction(ui->actionSkip_Forward);
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

    new QShortcut(QKeySequence(Qt::Key_J), this, [ = ] {
        this->rewind10();
    });
    new QShortcut(QKeySequence(Qt::Key_K), this, [ = ] {
        StateManager::instance()->playlist()->playPause();
    });
    new QShortcut(QKeySequence(Qt::Key_L), this, [ = ] {
        this->ff10();
    });
    new QShortcut(QKeySequence(Qt::Key_Up), this, [ = ] {
        double newVolume = StateManager::instance()->playlist()->volume();
        newVolume += 0.1;
        if (newVolume > 1) newVolume = 1;
        StateManager::instance()->playlist()->setVolume(newVolume);
    });
    new QShortcut(QKeySequence(Qt::Key_Down), this, [ = ] {
        double newVolume = StateManager::instance()->playlist()->volume();
        newVolume -= 0.1;
        if (newVolume < 0) newVolume = 0;
        StateManager::instance()->playlist()->setVolume(newVolume);
    });
    new QShortcut(QKeySequence(Qt::Key_Left), this, [ = ] {
        this->rewind10();
    });
    new QShortcut(QKeySequence(Qt::Key_Right), this, [ = ] {
        this->ff10();
    });

    QTimer::singleShot(0, this, [ = ] {
        resizeEvent(nullptr);
    });

#ifdef Q_OS_WIN
    new WinPlatformIntegration(this);

    QWinThumbnailToolBar* thumbBar = new QWinThumbnailToolBar(this);
    thumbBar->setWindow(this->windowHandle());

    QWinThumbnailToolButton* backToolButton = new QWinThumbnailToolButton(thumbBar);
    backToolButton->setToolTip(tr("Skip Back"));
    backToolButton->setIcon(QIcon::fromTheme("media-skip-backward"));
    backToolButton->setDismissOnClick(false);
    connect(backToolButton, &QWinThumbnailToolButton::clicked, this, [=] {
        StateManager::instance()->playlist()->previous();
    });

    QWinThumbnailToolButton* playPauseToolButton = new QWinThumbnailToolButton(thumbBar);
    playPauseToolButton->setToolTip(tr("Play"));
    playPauseToolButton->setIcon(QIcon::fromTheme("media-playback-start"));
    playPauseToolButton->setDismissOnClick(false);
    connect(playPauseToolButton, &QWinThumbnailToolButton::clicked, this, [=] {
        StateManager::instance()->playlist()->playPause();
    });
    connect(StateManager::instance()->playlist(), &Playlist::stateChanged, this, [=](Playlist::State state) {
        switch (state) {
            case Playlist::Playing:
                playPauseToolButton->setToolTip(tr("Pause"));
                playPauseToolButton->setIcon(QIcon::fromTheme("media-playback-pause"));
                break;
            case Playlist::Paused:
            case Playlist::Stopped:
                playPauseToolButton->setToolTip(tr("Play"));
                playPauseToolButton->setIcon(QIcon::fromTheme("media-playback-start"));
                break;
        }
    });

    QWinThumbnailToolButton* nextToolButton = new QWinThumbnailToolButton(thumbBar);
    nextToolButton->setToolTip(tr("Skip Next"));
    nextToolButton->setIcon(QIcon::fromTheme("media-skip-next"));
    nextToolButton->setDismissOnClick(false);
    connect(nextToolButton, &QWinThumbnailToolButton::clicked, this, [=] {
        StateManager::instance()->playlist()->next();
    });

    thumbBar->addButton(backToolButton);
    thumbBar->addButton(playPauseToolButton);
    thumbBar->addButton(nextToolButton);
#endif

    d->plugins.load();
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

void MainWindow::rewind10() {
    if (!StateManager::instance()->playlist()->currentItem()) return;

    qint64 seekTo = StateManager::instance()->playlist()->currentItem()->elapsed() - 10000;
    if (seekTo < 0) seekTo = 0;
    StateManager::instance()->playlist()->currentItem()->seek(seekTo);
}

void MainWindow::ff10() {
    if (!StateManager::instance()->playlist()->currentItem()) return;

    quint64 seekTo = StateManager::instance()->playlist()->currentItem()->elapsed() + 10000;
    if (seekTo > StateManager::instance()->playlist()->currentItem()->duration()) {
        StateManager::instance()->playlist()->next();
    } else {
        StateManager::instance()->playlist()->currentItem()->seek(seekTo);
    }
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

#include <QInputDialog>
void MainWindow::on_actionOpen_URL_triggered() {
    QString url = QInputDialog::getText(this, tr("Open URL"), tr("URL"));
    StateManager::instance()->playlist()->addItem(new QtMultimediaMediaItem(QUrl(url)));
}

void MainWindow::on_otherButton_toggled(bool checked) {
    if (checked) {
        ui->stackedWidget->setCurrentWidget(ui->otherSourcesPage);
    }
}

void MainWindow::on_actionSkip_Back_triggered() {
    StateManager::instance()->playlist()->previous();
}

void MainWindow::on_actionSkip_Forward_triggered() {
    StateManager::instance()->playlist()->next();
}

void MainWindow::on_actionPlayPause_triggered() {
    StateManager::instance()->playlist()->playPause();
}
