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
#include <tsettings.h>
#include <QDesktopServices>
#include <taboutdialog.h>
#include <QFileDialog>
#include <statemanager.h>
#include <playlist.h>
#include "library/librarymanager.h"
#include "playlistmodel.h"
#include <QTimer>
#include <QShortcut>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QInputDialog>
#include <thelpmenu.h>
#include <tjobmanager.h>
#include <sourcemanager.h>
#include "pluginmanager.h"
#include "settingsdialog.h"

#ifdef HAVE_THEINSTALLER
    #include "updatechecker.h"
#endif

#include <urlmanager.h>

#ifdef Q_OS_WIN
    #include <QWinThumbnailToolBar>
    #include <QWinThumbnailToolButton>
    #include "platformintegration/winplatformintegration.h"
#endif

struct MainWindowPrivate {
    tCsdTools csd;
    PluginManager plugins;

    tSettings settings;

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

    ui->jobButtonLayout->addWidget(tJobManager::makeJobButton());

    this->resize(SC_DPI_T(this->size(), QSize));

    ui->centralwidget->layout()->removeWidget(ui->topWidget);
    ui->topWidget->raise();
    ui->topWidget->move(0, 0);

    ui->menuBar->setVisible(false);
    ui->menuBar->addMenu(new tHelpMenu(this));

    QMenu* menu = new QMenu(this);

    tHelpMenu* helpMenu = new tHelpMenu(this);

#ifdef HAVE_THEINSTALLER
    if (UpdateChecker::updatesSupported()) {
        helpMenu->addAction(UpdateChecker::checkForUpdatesAction());

        connect(UpdateChecker::instance(), &UpdateChecker::updateAvailable, this, [ = ] {
            QPixmap menuPixmap = UpdateChecker::updateAvailableIcon(ui->menuButton->icon().pixmap(ui->menuButton->iconSize()));
            ui->menuButton->setIcon(QIcon(menuPixmap));

            QImage helpImage = helpMenu->icon().pixmap(SC_DPI_T(QSize(16, 16), QSize)).toImage();
            theLibsGlobal::tintImage(helpImage, this->palette().color(QPalette::WindowText));
            helpMenu->setIcon(QIcon(UpdateChecker::updateAvailableIcon(QPixmap::fromImage(helpImage))));
        });
    }
#endif

    menu->addAction(ui->actionOpen_File);
    menu->addAction(ui->actionOpen_URL);
    menu->addAction(ui->actionAdd_to_Library);
    menu->addSeparator();
    menu->addAction(ui->actionPlayPause);
    menu->addAction(ui->actionSkip_Back);
    menu->addAction(ui->actionSkip_Forward);
    menu->addSeparator();
    menu->addAction(ui->actionSettings);
//    menu->addMenu(helpMenu);
    menu->addMenu(helpMenu);
    menu->addAction(ui->actionExit);

#ifdef T_BLUEPRINT_BUILD
    ui->menuButton->setIcon(QIcon(":/icons/thebeat-blueprint.svg"));
#else
    ui->menuButton->setIcon(QIcon::fromTheme("thebeat", QIcon(":/icons/thebeat.svg")));
#endif
    ui->menuButton->setIconSize(SC_DPI_T(QSize(24, 24), QSize));
    ui->menuButton->setMenu(menu);
    ui->stackedWidget->setCurrentAnimation(tStackedWidget::SlideHorizontal);
    ui->queueStack->setCurrentAnimation(tStackedWidget::Fade);
    this->setWindowIcon(ui->menuButton->windowIcon());

    connect(StateManager::instance()->playlist(), &Playlist::itemsChanged, this, [ = ] {
        ui->queueStack->setCurrentWidget(StateManager::instance()->playlist()->items().isEmpty() ? ui->queuePromptPage : ui->queueListPage);

        //Sometimes the animation breaks it
        QTimer::singleShot(500, [ = ] {
            ui->queueStack->setCurrentWidget(StateManager::instance()->playlist()->items().isEmpty() ? ui->queuePromptPage : ui->queueListPage);
        });
    });
    ui->queueStack->setCurrentWidget(StateManager::instance()->playlist()->items().isEmpty() ? ui->queuePromptPage : ui->queueListPage);

    ui->artistsPage->setType(ArtistsAlbumsWidget::Artists);
    ui->albumsPage->setType(ArtistsAlbumsWidget::Albums);

    ui->queueWidget->installEventFilter(this);
    ui->queueWidget->setFixedWidth(SC_DPI(300));
    ui->queueWidget->setAcceptDrops(true);
    ui->queueList->setModel(new PlaylistModel);
    ui->queueList->setItemDelegate(new PlaylistDelegate());

    d->topBarLine = new QFrame(this);
    d->topBarLine->setFrameShape(QFrame::VLine);
    d->topBarLine->setFixedWidth(1);
    d->topBarLine->setParent(ui->topWidget);
    d->topBarLine->setVisible(true);
    d->topBarLine->lower();

    connect(new QShortcut(QKeySequence(Qt::Key_J), this), &QShortcut::activated, this, [ = ] {
        this->rewind10();
    });
    connect(new QShortcut(QKeySequence(Qt::Key_K), this), &QShortcut::activated, this, [ = ] {
        StateManager::instance()->playlist()->playPause();
    });
    connect(new QShortcut(QKeySequence(Qt::Key_L), this), &QShortcut::activated, this, [ = ] {
        this->ff10();
    });
    connect(new QShortcut(QKeySequence(Qt::Key_Up), this), &QShortcut::activated, this, [ = ] {
        double newVolume = StateManager::instance()->playlist()->volume();
        newVolume += 0.1;
        if (newVolume > 1) newVolume = 1;
        StateManager::instance()->playlist()->setVolume(newVolume);
    });
    connect(new QShortcut(QKeySequence(Qt::Key_Down), this), &QShortcut::activated, this, [ = ] {
        double newVolume = StateManager::instance()->playlist()->volume();
        newVolume -= 0.1;
        if (newVolume < 0) newVolume = 0;
        StateManager::instance()->playlist()->setVolume(newVolume);
    });
    connect(new QShortcut(QKeySequence(Qt::Key_Left), this), &QShortcut::activated, this, [ = ] {
        this->rewind10();
    });
    connect(new QShortcut(QKeySequence(Qt::Key_Right), this), &QShortcut::activated, this, [ = ] {
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
    connect(backToolButton, &QWinThumbnailToolButton::clicked, this, [ = ] {
        StateManager::instance()->playlist()->previous();
    });

    QWinThumbnailToolButton* playPauseToolButton = new QWinThumbnailToolButton(thumbBar);
    playPauseToolButton->setToolTip(tr("Play"));
    playPauseToolButton->setIcon(QIcon::fromTheme("media-playback-start"));
    playPauseToolButton->setDismissOnClick(false);
    connect(playPauseToolButton, &QWinThumbnailToolButton::clicked, this, [ = ] {
        StateManager::instance()->playlist()->playPause();
    });
    connect(StateManager::instance()->playlist(), &Playlist::stateChanged, this, [ = ](Playlist::State state) {
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
    nextToolButton->setIcon(QIcon::fromTheme("media-skip-forward"));
    nextToolButton->setDismissOnClick(false);
    connect(nextToolButton, &QWinThumbnailToolButton::clicked, this, [ = ] {
        StateManager::instance()->playlist()->next();
    });

    thumbBar->addButton(backToolButton);
    thumbBar->addButton(playPauseToolButton);
    thumbBar->addButton(nextToolButton);
#endif

    connect(&d->settings, &tSettings::settingChanged, this, [ = ](QString key, QVariant value) {
        if (key == "notifications/trackChange") {
            StateManager::instance()->playlist()->setTrachChangeNotificationsEnabled(value.toBool());
        } else if (key == "appearance/useSsds") {
            tCsdGlobal::setCsdsEnabled(!value.toBool());
        }
    });
    StateManager::instance()->playlist()->setTrachChangeNotificationsEnabled(d->settings.value("notifications/trackChange").toBool());
    tCsdGlobal::setCsdsEnabled(!d->settings.value("appearance/useSsds").toBool());

    StateManager::instance()->sources()->setPadTop(ui->topWidget->sizeHint().height());

    d->plugins.load();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_actionOpen_File_triggered() {
    QFileDialog* dialog = new QFileDialog(this);
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->setFileMode(QFileDialog::ExistingFiles);
    connect(dialog, &QFileDialog::filesSelected, this, [ = ](QStringList files) {
        for (QString file : files) {
            MediaItem* item = StateManager::instance()->url()->itemForUrl(QUrl::fromLocalFile(file));
            StateManager::instance()->playlist()->addItem(item);
        }
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

    ui->topWidget->setFixedWidth(ui->centralwidget->width());
    ui->topWidget->setFixedHeight(ui->topWidget->sizeHint().height());
    ui->tracksPage->setTopPadding(ui->topWidget->height());
    ui->artistsPage->setTopPadding(ui->topWidget->height());
    ui->albumsPage->setTopPadding(ui->topWidget->height());
    ui->playlistsPage->setTopPadding(ui->topWidget->height());
    ui->otherSourcesPage->setTopPadding(ui->topWidget->height());
    ui->queueWidget->setContentsMargins(0, ui->topWidget->height(), 0, 0);
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (watched == ui->queueWidget) {
        if (event->type() == QEvent::DragEnter) {
            QDragEnterEvent* e = static_cast<QDragEnterEvent*>(event);
            if (e->mimeData()->hasUrls()) {
                e->setDropAction(Qt::CopyAction);
                e->acceptProposedAction();
            }
            return true;
        } else if (event->type() == QEvent::Drop) {
            QDropEvent* e = static_cast<QDropEvent*>(event);
            e->setDropAction(Qt::CopyAction);
            if (e->mimeData()->hasUrls()) {
                for (QUrl url : e->mimeData()->urls()) {
                    StateManager::instance()->playlist()->addItem(StateManager::instance()->url()->itemForUrl(url));
                }
                e->acceptProposedAction();
            }
            return true;
        }
    }
    return false;
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
    //Check if the user is trying to select multiple items
    if (QApplication::keyboardModifiers() & Qt::ControlModifier || QApplication::keyboardModifiers() & Qt::ShiftModifier) return;

    StateManager::instance()->playlist()->setCurrentItem(index.data(PlaylistModel::MediaItemRole).value<MediaItem*>());
}

void MainWindow::on_queueList_customContextMenuRequested(const QPoint& pos) {
    QMenu* menu = new QMenu(this);

    QModelIndexList selected = ui->queueList->selectionModel()->selectedIndexes();
    for (int i = 0; i != selected.count(); i++) {
        if (selected.at(i).data(PlaylistModel::DrawTypeRole).value<PlaylistModel::DrawType>() == PlaylistModel::GroupHeader) {
            selected.removeAt(i);
            i--;
        }
    }

    if (!selected.isEmpty()) {
        if (selected.count() == 1) {
            menu->addSection(tr("For \"%1\"").arg(menu->fontMetrics().elidedText(selected.first().data(Qt::DisplayRole).toString(), Qt::ElideMiddle, SC_DPI(300))));
        } else {
            menu->addSection(tr("For %n items", nullptr, selected.count()));
        }
        menu->addAction(QIcon::fromTheme("list-remove"), tr("Remove from Queue"), [ = ] {
            //Directly removing the items causes the list to change and invalidate itself
            QList<MediaItem*> itemsToRemove;
            for (QModelIndex idx : selected) {
                itemsToRemove.append(idx.data(PlaylistModel::MediaItemRole).value<MediaItem*>());
            }

            for (MediaItem* item : itemsToRemove) {
                StateManager::instance()->playlist()->removeItem(item);
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

void MainWindow::on_actionOpen_URL_triggered() {
    bool ok;
    QString url = QInputDialog::getText(this, tr("Open URL"), tr("URL"), QLineEdit::Normal, "", &ok);

    if (ok) {
        StateManager::instance()->playlist()->addItem(StateManager::instance()->url()->itemForUrl(QUrl(url)));
    }
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

void MainWindow::on_playlistsButton_toggled(bool checked) {
    if (checked) {
        ui->stackedWidget->setCurrentWidget(ui->playlistsPage);
    }
}

void MainWindow::on_actionAdd_to_Library_triggered() {
    QFileDialog* dialog = new QFileDialog(this);
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->setFileMode(QFileDialog::Directory);
    dialog->setOption(QFileDialog::ShowDirsOnly);
    connect(dialog, &QFileDialog::filesSelected, this, [ = ](QStringList files) {
        for (QString file : files) {
            LibraryManager::instance()->enumerateDirectory(file, true, true);
        }
    });
    connect(dialog, &QFileDialog::finished, dialog, &QFileDialog::deleteLater);
    dialog->open();
}

void MainWindow::on_actionSettings_triggered() {
    SettingsDialog dialog;
    dialog.exec();
}

void MainWindow::on_actionHelp_triggered() {
    QDesktopServices::openUrl(QUrl("https://help.vicr123.com/docs/thebeat/intro"));
}
