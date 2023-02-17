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

#include "commandpalette/artistsalbumscommandpalettescope.h"
#include "commandpalette/trackscommandpalettescope.h"
#include "library/librarymanager.h"
#include "playlistmodel.h"
#include "print/printcontroller.h"
#include "settingspanes/colourssettingspane.h"
#include "settingspanes/libraryresetsettingspane.h"
#include "settingspanes/notificationssettingspane.h"
#include "settingspanes/titlebarsettingspane.h"
#include "twindowthumbnail.h"
#include <QDesktopServices>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QMenu>
#include <QMimeData>
#include <QShortcut>
#include <QTimer>
#include <abstractlibrarybrowser.h>
#include <playlist.h>
#include <plugins/tpluginmanagerpane.h>
#include <sourcemanager.h>
#include <statemanager.h>
#include <taboutdialog.h>
#include <tapplication.h>
#include <tcommandpalette/tcommandpaletteactionscope.h>
#include <tcommandpalette/tcommandpalettecontroller.h>
#include <tcsdtools.h>
#include <thelpmenu.h>
#include <ticon.h>
#include <tinputdialog.h>
#include <tjobmanager.h>
#include <tmessagebox.h>
#include <tpopover.h>
#include <tsettings.h>
#include <tsettingswindow/tsettingswindow.h>
#include <tstylemanager.h>

#ifdef HAVE_THEINSTALLER
    #include "updatechecker.h"
#endif

#include <urlmanager.h>

struct MainWindowPrivate {
        tCsdTools csd;

    tSettings settings;

    QFrame* topBarLine;
};

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    StateManager::instance()->setMainWindow(this);

    d = new MainWindowPrivate();

    d->topBarLine = new QFrame(this);
    d->topBarLine->setFrameShape(QFrame::VLine);
    d->topBarLine->setFixedWidth(1);
    d->topBarLine->setParent(ui->topWidget);
    d->topBarLine->setVisible(true);
    d->topBarLine->lower();

    d->csd.installMoveAction(ui->topWidget);
#ifndef Q_OS_MAC
    d->csd.installResizeAction(this);
#endif

    if (tCsdGlobal::windowControlsEdge() == tCsdGlobal::Left) {
        ui->leftCsdLayout->addWidget(d->csd.csdBoxForWidget(this));
    } else {
        ui->rightCsdLayout->addWidget(d->csd.csdBoxForWidget(this));
    }

    ui->jobButtonLayout->addWidget(tJobManager::makeJobButton());

    this->resize(SC_DPI_T(this->size(), QSize));

    tCommandPaletteActionScope* commandPaletteActionScope;
    auto commandPalette = tCommandPaletteController::defaultController(this, &commandPaletteActionScope);

    auto artistCommandPaletteScope = new ArtistsAlbumsCommandPaletteScope(true, this);
    connect(artistCommandPaletteScope, &ArtistsAlbumsCommandPaletteScope::activated, this, [this](QString text) {
        ui->stackedWidget->setCurrentWidget(ui->artistsPage);
        ui->artistsPage->changeItem(text);
    });
    commandPalette->addScope(artistCommandPaletteScope);

    auto albumCommandPaletteScope = new ArtistsAlbumsCommandPaletteScope(false, this);
    connect(albumCommandPaletteScope, &ArtistsAlbumsCommandPaletteScope::activated, this, [this](QString text) {
        ui->stackedWidget->setCurrentWidget(ui->albumsPage);
        ui->albumsPage->changeItem(text);
    });
    commandPalette->addScope(albumCommandPaletteScope);

    commandPalette->addScope(new TracksCommandPaletteScope(this));

    ui->centralwidget->layout()->removeWidget(ui->topWidget);
    ui->topWidget->raise();
    ui->topWidget->move(0, 0);

    ui->windowTabber->addButton(new tWindowTabberButton(QIcon::fromTheme("view-media-track"), tr("Tracks"), ui->stackedWidget, ui->tracksPage));
    ui->windowTabber->addButton(new tWindowTabberButton(QIcon::fromTheme("view-media-artist"), tr("Artists"), ui->stackedWidget, ui->artistsPage));
    ui->windowTabber->addButton(new tWindowTabberButton(QIcon::fromTheme("media-album-cover"), tr("Albums"), ui->stackedWidget, ui->albumsPage));
    ui->windowTabber->addButton(new tWindowTabberButton(QIcon::fromTheme("view-media-playlist"), tr("Playlists"), ui->stackedWidget, ui->playlistsPage));
    ui->windowTabber->addButton(new tWindowTabberButton(QIcon::fromTheme("view-list-details"), tr("Other Sources"), ui->stackedWidget, ui->otherSourcesPage));

    ui->menuBar->addMenu(new tHelpMenu(this, commandPalette));
#ifdef Q_OS_MAC
    ui->menuButton->setVisible(false);
#else
    ui->menuBar->setVisible(false);
    QMenu* menu = new QMenu(this);

    tHelpMenu* helpMenu = new tHelpMenu(this);

#ifdef HAVE_THEINSTALLER
    if (tApplication::currentPlatform() != tApplication::WindowsAppPackage && UpdateChecker::updatesSupported()) {
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
    menu->addAction(ui->actionRepeat_One);
    menu->addAction(ui->actionShuffle);
    menu->addSeparator();
    menu->addAction(ui->actionPrint);
    menu->addSeparator();
    menu->addAction(commandPalette->commandPaletteAction());
    menu->addAction(ui->actionSettings);
    menu->addMenu(helpMenu);
    menu->addAction(ui->actionExit);

    ui->menuButton->setIcon(tApplication::applicationIcon());
    ui->menuButton->setIconSize(QSize(24, 24));
    ui->menuButton->setMenu(menu);
#endif
    commandPaletteActionScope->addMenuBar(ui->menuBar);

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::SlideHorizontal);
    ui->queueStack->setCurrentAnimation(tStackedWidget::Fade);
    this->setWindowIcon(ui->menuButton->windowIcon());

    connect(StateManager::instance()->playlist(), &Playlist::itemsChanged, this, [this] {
        ui->queueStack->setCurrentWidget(StateManager::instance()->playlist()->items().isEmpty() ? ui->queuePromptPage : ui->queueListPage);

        // Sometimes the animation breaks it
        QTimer::singleShot(500, [this] {
            ui->queueStack->setCurrentWidget(StateManager::instance()->playlist()->items().isEmpty() ? ui->queuePromptPage : ui->queueListPage);
        });
    });
    ui->queueStack->setCurrentWidget(StateManager::instance()->playlist()->items().isEmpty() ? ui->queuePromptPage : ui->queueListPage);

    ui->artistsPage->setType(ArtistsAlbumsWidget::Artists);
    ui->albumsPage->setType(ArtistsAlbumsWidget::Albums);

    ui->queueWidget->installEventFilter(this);
    ui->queueWidget->setFixedWidth(300);
    ui->queueWidget->setAcceptDrops(true);
    ui->queueList->setModel(new PlaylistModel);
    ui->queueList->setItemDelegate(new PlaylistDelegate());

    connect(new QShortcut(QKeySequence(Qt::Key_J), this), &QShortcut::activated, this, [this] {
        this->rewind10();
    });
    connect(new QShortcut(QKeySequence(Qt::Key_K), this), &QShortcut::activated, this, [] {
        StateManager::instance()->playlist()->playPause();
    });
    connect(new QShortcut(QKeySequence(Qt::Key_L), this), &QShortcut::activated, this, [this] {
        this->ff10();
    });
    connect(new QShortcut(QKeySequence(Qt::Key_Up), this), &QShortcut::activated, this, [this] {
        on_actionIncrease_Volume_triggered();
    });
    connect(new QShortcut(QKeySequence(Qt::Key_Down), this), &QShortcut::activated, this, [this] {
        on_actionDecrease_Volume_triggered();
    });
    connect(new QShortcut(QKeySequence(Qt::Key_Left), this), &QShortcut::activated, this, [this] {
        this->rewind10();
    });
    connect(new QShortcut(QKeySequence(Qt::Key_Right), this), &QShortcut::activated, this, [this] {
        this->ff10();
    });

    QTimer::singleShot(0, this, [this] {
        resizeEvent(nullptr);
    });

    auto thumbnail = tWindowThumbnail::thumbnailFor(this);

    if (thumbnail) {
        thumbnail->setToolbar(QList<QAction*>{ui->actionSkip_Back, ui->actionPlayPause, ui->actionSkip_Forward});
    }

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

    connect(StateManager::instance()->playlist(), &Playlist::repeatOneChanged, this, [this](bool repeatOne) {
        ui->actionRepeat_One->setChecked(repeatOne);
    });
    connect(StateManager::instance()->playlist(), &Playlist::shuffleChanged, this, [this](bool shuffle) {
        ui->actionShuffle->setChecked(shuffle);
    });
    connect(StateManager::instance()->playlist(), &Playlist::itemsChanged, this, [this] {
        updatePlayState();
    });
    connect(StateManager::instance()->playlist(), &Playlist::stateChanged, this, [this] {
        updatePlayState();
    });
    updatePlayState();

    connect(ui->controlStrip, &ControlStrip::inZenModeChanged, this, [this](bool inZenMode) {
        ui->actionZen_Mode->setChecked(inZenMode);
    });

    setWindowIcon(tApplication::applicationIcon());

    tIcon::processWidget(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::show() {
    QMainWindow::show();
#ifdef Q_OS_MAC
    d->csd.installResizeAction(this);
#endif
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
    dialog->setAttribute(Qt::WA_DeleteOnClose);
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

void MainWindow::closeEvent(QCloseEvent* event) {
#ifdef Q_OS_MAC
    d->csd.removeResizeAction(this);
    this->hide();
    event->accept();
#endif
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

void MainWindow::updatePlayState() {
    if (StateManager::instance()->playlist()->items().length() == 0) {
        ui->actionPlayPause->setEnabled(false);
        ui->actionSkip_Back->setEnabled(false);
        ui->actionSkip_Forward->setEnabled(false);
        ui->actionZen_Mode->setEnabled(false);
    } else {
        ui->actionPlayPause->setEnabled(true);
        ui->actionSkip_Back->setEnabled(true);
        ui->actionSkip_Forward->setEnabled(true);
        ui->actionZen_Mode->setEnabled(true);
    }

    if (StateManager::instance()->playlist()->state() == Playlist::Playing) {
        ui->actionPlayPause->setText(tr("Pause"));
        ui->actionPlayPause->setIcon(QIcon::fromTheme("media-playback-pause"));
    } else {
        ui->actionPlayPause->setText(tr("Play"));
        ui->actionPlayPause->setIcon(QIcon::fromTheme("media-playback-start"));
    }
}

void MainWindow::on_queueList_activated(const QModelIndex& index) {
    // Check if the user is trying to select multiple items
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
            // Directly removing the items causes the list to change and invalidate itself
            QList<MediaItem*> itemsToRemove;
            for (const QModelIndex& idx : selected) {
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
    QString url = tInputDialog::getText(this, tr("Open URL"), tr("Enter the URL you'd like to open"), QLineEdit::Normal, "", &ok);

    if (ok) {
        MediaItem* item = StateManager::instance()->url()->itemForUrl(QUrl(url));
        if (!item) {
            tMessageBox messageBox(this);
            messageBox.setTitleBarText(tr("Can't open that URL"));
            messageBox.setMessageText(tr("Sorry, that URL isn't supported by theBeat."));
            messageBox.setIcon(QMessageBox::Information);
            messageBox.exec();
        } else {
            StateManager::instance()->playlist()->addItem(item);
        }
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
    tSettingsWindow window(this);
    window.appendSection(tr("General"));
    window.appendPane(new NotificationsSettingsPane());
    window.appendPane(new tPluginManagerPane());

    window.appendSection(tr("Appearance"));
    window.appendPane(new TitlebarSettingsPane());

    if (tStyleManager::isOverridingStyle()) {
        window.appendPane(new ColoursSettingsPane());
    }

    window.appendSection(tr("Library"));
    window.appendPane(new LibraryResetSettingsPane());
    window.exec();
}

void MainWindow::on_actionHelp_triggered() {
    QDesktopServices::openUrl(QUrl("https://help.vicr123.com/docs/thebeat/intro"));
}

void MainWindow::on_actionRepeat_One_triggered(bool checked) {
    StateManager::instance()->playlist()->setRepeatOne(checked);
}

void MainWindow::on_actionShuffle_triggered(bool checked) {
    StateManager::instance()->playlist()->setShuffle(checked);
}

void MainWindow::on_actionPrint_triggered() {
    if (!PrintController::hasPrintersAvailable()) {
        auto box = new tMessageBox(this);
        box->setTitleBarText(tr("No Printers"));
        box->setMessageText(tr("Before printing a list of tracks, you'll need to set up a printer."));
        box->show(true);
        return;
    }

    AbstractLibraryBrowser* currentBrowser = qobject_cast<AbstractLibraryBrowser*>(ui->stackedWidget->currentWidget());
    if (!currentBrowser || currentBrowser->currentListInformation().tracks.isEmpty()) {
        tMessageBox* box = new tMessageBox(this);
        box->setIcon(QMessageBox::Information);
        box->setTitleBarText(tr("Print"));
        box->setMessageText(tr("Open a list of tracks (for example, a playlist) to print it."));
        box->show(true);
    } else {
        PrintController* controller = new PrintController(currentBrowser->currentListInformation(), this);
        controller->confirmAndPerformPrint();
    }
}

void MainWindow::on_actionZen_Mode_triggered() {
    if (ui->controlStrip->inZenMode()) {
        ui->controlStrip->leaveZenMode();
    } else {
        ui->controlStrip->enterZenMode();
    }
}

void MainWindow::on_actionIncrease_Volume_triggered() {
    double newVolume = StateManager::instance()->playlist()->volume();
    newVolume += 0.1;
    if (newVolume > 1) newVolume = 1;
    StateManager::instance()->playlist()->setVolume(newVolume);
}

void MainWindow::on_actionDecrease_Volume_triggered() {
    double newVolume = StateManager::instance()->playlist()->volume();
    newVolume -= 0.1;
    if (newVolume < 0) newVolume = 0;
    StateManager::instance()->playlist()->setVolume(newVolume);
}
