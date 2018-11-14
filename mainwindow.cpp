#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QScroller>
#include <QInputDialog>
#include <QMessageBox>
#include <QShortcut>
#include <QSvgRenderer>
#include <QGraphicsBlurEffect>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include "secondaryinformationlistdelegate.h"

#ifdef Q_OS_LINUX
    #include <X11/Xlib.h>
    #include <QX11Info>

    #undef FocusOut
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->currentMediaFrame->setVisible(false);
    this->resize(this->size() * theLibsGlobal::getDPIScaling());
    ui->sourcesList->setIconSize(ui->sourcesList->iconSize() * theLibsGlobal::getDPIScaling());

    libWidget = new LibraryManageWidget();
    ui->libStack->addWidget(libWidget);
    connect(libWidget, &LibraryManageWidget::editingDone, [=] {
        ui->libStack->setCurrentIndex(library->reloadData());
    });

#ifdef BUILD_DISCORD
    discord = new DiscordIntegration();
#endif

    player = new MediaObject(this);
    cdFinder = new MediaObject(this);
    dataOut = new AudioDataOutput(this);
    playlist = new PlaylistModel(player);
    controller = new MediaController(player);
    cdController = new MediaController(cdFinder);
    temporaryMprisAlbumArt = new QTemporaryFile();

    player->setTickInterval(1000);

    createPath(player, new AudioOutput(Phonon::MusicCategory, this));
    connect(player, SIGNAL(metaDataChanged()), this, SLOT(updateMetadata()));
    //connect(player, SIGNAL(aboutToFinish()), this, SLOT(playerAboutToFinish()));
    connect(player, SIGNAL(aboutToFinish()), playlist, SLOT(enqueueNext()));
    connect(player, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(playerStateChanged(Phonon::State,Phonon::State)));
    connect(player, SIGNAL(currentSourceChanged(Phonon::MediaSource)), this, SLOT(updateMetadata()));
    connect(player, &MediaObject::totalTimeChanged, [=](qint64 totalTime) {
        mprisMetadataMap.insert("mpris:length", totalTime * 1000);

        if (totalTime == 0) {
            ui->totalTime->setText("∞");
        } else {
            QDateTime t = QDateTime::fromMSecsSinceEpoch(totalTime);
            if (t.time().hour() == 0) {
                ui->totalTime->setText(t.toString("HH:mm:ss"));
            } else {
                ui->totalTime->setText(t.toString("mm:ss"));
            }
        }
    });
    connect(player, &MediaObject::tick, [=](qint64 time) {
        QDateTime t = QDateTime::fromMSecsSinceEpoch(time);
        if (t.time().hour() == 0) {
            ui->elapsedTime->setText(t.toString("HH:mm:ss"));
        } else {
            ui->elapsedTime->setText(t.toString("mm:ss"));
        }
    });

    dataOut->setDataSize(ui->visualisationFrame->width());
    connect(dataOut, SIGNAL(dataReady(QMap<Phonon::AudioDataOutput::Channel,QVector<qint16>>)), this, SLOT(dataDecoded(QMap<Phonon::AudioDataOutput::Channel,QVector<qint16>>)));
    createPath(player, dataOut);

    ui->seeker->setMediaObject(player);

    ui->widget->setVisible(false);
    ui->label_9->setVisible(false);
    ui->searchWidget->setFixedWidth(0);
    ui->searchWidget->setVisible(false);
    ui->searchLineEdit->installEventFilter(this);
    ui->libraryBackButton->setVisible(false);
    ui->sourcesStack->setCurrentIndex(1);
    ui->mediaLibraryInfoWidget->installEventFilter(this);

    ui->playlistWidget->setModel(playlist);

    QShortcut* spaceShortcut = new QShortcut(QKeySequence(Qt::Key_Space), this);
    spaceShortcut->setAutoRepeat(false);
    connect(spaceShortcut, &QShortcut::activated, [=] {
        ui->playButton->click();
    });

    QMenu* playlistMenu = new QMenu();
    playlistMenu->addSection(tr("For current playlist"));
    playlistMenu->addAction(ui->actionClear_Playlist);
    playlistMenu->addAction(ui->actionSave_Playlist);
    playlistMenu->addAction(ui->actionAdd_to_existing_playlist);
    ui->playlistMenuButton->setMenu(playlistMenu);

    new MediaPlayer2Adaptor(this);
    new PlayerAdaptor(this);

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/org/mpris/MediaPlayer2", this, QDBusConnection::ExportAdaptors);
    dbus.registerService("org.mpris.MediaPlayer2.theBeat");

    library = new LibraryModel;
    ui->library->setModel(library);
    ui->library->header()->setStretchLastSection(false);
    ui->library->header()->setDefaultSectionSize(300 * theLibsGlobal::getDPIScaling());
    //ui->library->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->library->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->library->header()->setSectionResizeMode(1, QHeaderView::Interactive);
    ui->library->header()->setSectionResizeMode(2, QHeaderView::Interactive);
    ui->library->header()->setSortIndicatorShown(true);
    ui->library->setSortingEnabled(true);
    ui->library->setItemDelegateForColumn(0, new LibraryTitleDelegate(this));
    ui->library->sortByColumn(0, Qt::DescendingOrder);
    connect(ui->library->header(), &QHeaderView::sortIndicatorChanged, [=](int index, Qt::SortOrder order) {
        ui->library->sortByColumn(index, order);
    });
    ui->libraryTrackNumber->setText(tr("%n tracks", nullptr, library->rowCount()));
    ui->libStack->setCurrentIndex(library->reloadData());

    ArtistLibraryModel* artistModel = new ArtistLibraryModel(library);
    ui->artistsView->setModel(artistModel);
    ui->artistsView->setItemDelegate(new SecondaryInformationListDelegate);

    AlbumLibraryModel* albumModel = new AlbumLibraryModel(library);
    ui->albumsView->setModel(albumModel);
    ui->albumsView->setItemDelegate(new SecondaryInformationListDelegate);

    ui->PlaylistsView->setModel(new PlaylistFileModel);

    AudioDataOutput* dummyOutput = new AudioDataOutput(this);
    createPath(cdFinder, dummyOutput);
    connect(cdController, &MediaController::availableTitlesChanged, [=](int numberOfTracks) {
        //New CD inserted
        qDebug() << numberOfTracks;
    });
    cdFinder->setCurrentSource(MediaSource(Phonon::Cd, "/dev/sr0"));
    cdFinder->play();
    cdFinder->pause();

    ui->appTitleLabel->setFixedHeight(ui->appTitleLabel->fontMetrics().height() + 18); //Don't scale DPI because margins don't scale

    //Allow dragging with finger
    QScroller::grabGesture(ui->sourcesList, QScroller::LeftMouseButtonGesture);
    QScroller::grabGesture(ui->library, QScroller::LeftMouseButtonGesture);
    QScroller::grabGesture(ui->PlaylistsView, QScroller::LeftMouseButtonGesture);
    QScroller::grabGesture(ui->playlistWidget, QScroller::LeftMouseButtonGesture);
    QScroller::grabGesture(ui->artistsView, QScroller::LeftMouseButtonGesture);
    QScroller::grabGesture(ui->albumsView, QScroller::LeftMouseButtonGesture);
}

MainWindow::~MainWindow()
{
    delete ui;
    temporaryMprisAlbumArt->deleteLater();
}

void MainWindow::on_actionOpen_triggered()
{
    QFileDialog* dialog = new QFileDialog();
    dialog->setFileMode(QFileDialog::ExistingFiles);
    if (dialog->exec() == QDialog::Accepted) {
        for (QUrl file : dialog->selectedUrls()) {
            MediaSource source(file);
            playlist->append(source);
        }

        playlist->enqueueAndPlayNext();
    }

    dialog->deleteLater();
}

void MainWindow::updateMetadata() {
    QVariantMap currentDataMap = mprisMetadataMap;
    bool showChangedNotification = false;
    QStringList metadata;

    QString title, artist;

    QStringList Title = player->metaData(Phonon::TitleMetaData);
    if (Title.count() > 0) {
        if (Title.at(0) != ui->currentTitleLabel->text()) {
            title = Title.at(0);
            ui->currentTitleLabel->setText(title);
            showChangedNotification = true;
        }
        mprisMetadataMap.insert("xesam:title", Title.first());
    } else {
        title = playlist->data(playlist->index(playlist->currentItem())).toString();
        ui->currentTitleLabel->setText(title);
        mprisMetadataMap.insert("xesam:title", title);
    }

    QStringList Artist = player->metaData(Phonon::ArtistMetaData);
    if (Artist.count() > 0) {
        artist = Artist.at(0);
        metadata.append(artist);
        mprisMetadataMap.insert("xesam:artist", Artist);
    } else {
        mprisMetadataMap.remove("xesam:artist");
    }

    QStringList Album = player->metaData(Phonon::AlbumMetaData);
    if (Album.count() > 0) {
        metadata.append(Album.at(0));
        mprisMetadataMap.insert("xesam:album", Album.first());
    } else {
        mprisMetadataMap.remove("xesam:album");
    }

    ui->currentMetadataLabel->setText(metadata.join(" · "));
    ui->currentMediaFrame->setVisible(true);

    temporaryMprisAlbumArt->deleteLater();
    temporaryMprisAlbumArt = new QTemporaryFile("thebeat-albumart-XXXXXX.png");
    QImage i = TagCache::getAlbumArt(player->currentSource().fileName());
    if (i.isNull()) {
        ui->albumArtLabel->setPixmap(QIcon::fromTheme("audio").pixmap(48 * theLibsGlobal::getDPIScaling(), 48 * theLibsGlobal::getDPIScaling()));
        mprisMetadataMap.remove("mpris:artUrl");
        ui->currentMediaFrame->setPalette(this->palette());
    } else {
        //Write MPRIS data out
        temporaryMprisAlbumArt->open();
        i.save(temporaryMprisAlbumArt, "PNG");
        mprisMetadataMap.insert("mpris:artUrl", QUrl::fromLocalFile(temporaryMprisAlbumArt->fileName()).toString());

        //Set image
        QImage image = i.scaled(48 * theLibsGlobal::getDPIScaling(), 48 * theLibsGlobal::getDPIScaling(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        qulonglong red = 0, green = 0, blue = 0;

        QPalette pal = this->palette();
        int totalPixels = 0;
        for (int i = 0; i < image.width(); i++) {
            for (int j = 0; j < image.height(); j++) {
                QColor c = image.pixelColor(i, j);
                if (c.alpha() != 0) {
                    red += c.red();
                    green += c.green();
                    blue += c.blue();
                    totalPixels++;
                }
            }
        }

        QColor c;
        int averageCol = (pal.color(QPalette::Window).red() + pal.color(QPalette::Window).green() + pal.color(QPalette::Window).blue()) / 3;

        if (totalPixels == 0) {
            if (averageCol < 127) {
                c = pal.color(QPalette::Window).darker(200);
            } else {
                c = pal.color(QPalette::Window).lighter(200);
            }
        } else {
            c = QColor(red / totalPixels, green / totalPixels, blue / totalPixels);

            if (averageCol < 127) {
                c = c.darker(200);
            } else {
                c = c.lighter(200);
            }
        }

        pal.setColor(QPalette::Window, c);
        ui->currentMediaFrame->setPalette(pal);

        QImage rounded(48 * theLibsGlobal::getDPIScaling(), 48 * theLibsGlobal::getDPIScaling(), QImage::Format_ARGB32);
        rounded.fill(Qt::transparent);
        QPainter p(&rounded);
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(QBrush(image));
        p.setPen(Qt::transparent);
        p.drawRoundedRect(0, 0, 48 * theLibsGlobal::getDPIScaling(), 48 * theLibsGlobal::getDPIScaling(), 40, 40, Qt::RelativeSize);

        ui->albumArtLabel->setPixmap(QPixmap::fromImage(rounded));
    }

    if (showChangedNotification) {
        tNotification* notification = new tNotification();
        notification->setSummary("Now Playing");

        QStringList notificationText;
        notificationText.append(title);
        notificationText.append(artist);

        notification->setText(notificationText.join(" · "));
        notification->setSoundOn(false);
        notification->setAppName("theBeat");
        notification->setTransient(true);
        notification->post();
    }

    mprisMetadataMap.insert("mpris:trackid", QVariant::fromValue(QDBusObjectPath("/org/thesuite/thebeat/currentmedia")));

    if (currentDataMap != mprisMetadataMap) {
        //Send the PropertiesChanged signal.
        QDBusMessage signal = QDBusMessage::createSignal("/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "PropertiesChanged");

        QList<QVariant> args;
        args.append("org.mpris.MediaPlayer2.Player");

        QVariantMap changedProperties;
        changedProperties.insert("Metadata", mprisMetadataMap);
        //changedProperties.insert("PlaybackStatus", this->PlaybackStatus());
        args.append(changedProperties);

        QStringList invalidatedProperties;
        invalidatedProperties.append("Metadata");
        //invalidatedProperties.append("PlaybackStatus");
        args.append(invalidatedProperties);

        signal.setArguments(args);

        QDBusConnection::sessionBus().send(signal);
    }
}

void MainWindow::on_playButton_clicked()
{
    if (player->state() == PlayingState) {
        player->pause();
    } else {
        player->play();
    }
}

void MainWindow::on_nextButton_clicked()
{
    playlist->playNext();
}

void MainWindow::on_backButton_clicked()
{
    playlist->skipBack();
}

void MainWindow::playerAboutToFinish() {
}

void MainWindow::playerStateChanged(State newState, State oldState) {
    QString status;
    if (newState == PlayingState) {
        ui->playButton->setIcon(QIcon::fromTheme("media-playback-pause"));
        ui->playButton->setToolTip(tr("Pause"));
        status = "Playing";
    } else if (newState == PausedState || newState == BufferingState) {
        ui->playButton->setIcon(QIcon::fromTheme("media-playback-start"));
        ui->playButton->setToolTip(tr("Play"));
        status = "Paused";
    } else {
        ui->currentMediaFrame->setVisible(false);
        status = "Stopped";
    }

    //Send the PropertiesChanged signal.
    QDBusMessage signal = QDBusMessage::createSignal("/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "PropertiesChanged");

    QList<QVariant> args;
    args.append("org.mpris.MediaPlayer2.Player");

    QVariantMap changedProperties;
    changedProperties.insert("PlaybackStatus", status);
    args.append(changedProperties);

    QStringList invalidatedProperties;
    invalidatedProperties.append("PlaybackStatus");
    args.append(invalidatedProperties);

    signal.setArguments(args);

    QDBusConnection::sessionBus().send(signal);
}

void MainWindow::on_playlistWidget_activated(const QModelIndex &index)
{
    playlist->playItem(index.row());
}

QVariantMap MainWindow::metadataMap() {
    return mprisMetadataMap;
}

MediaObject* MainWindow::getPlayer() {
    return player;
}

void MainWindow::on_sourcesList_activated(const QModelIndex &index)
{
    /*switch (index.row()) {
        case 2: //Open File
            ui->actionOpen->trigger();
    }*/
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_repeatButton_toggled(bool checked)
{
    playlist->setRepeat(checked);
}

void MainWindow::on_sourcesList_currentRowChanged(int currentRow)
{
    /*switch (currentRow) {
        case 0:
            ui->sourcesStack->setCurrentIndex(0);
            break;
        case 3:
            ui->sourcesStack->setCurrentIndex(2);
            break;
    }*/
    ui->sourcesStack->setCurrentIndex(currentRow);
}

void MainWindow::on_AddNetworkStreamButton_clicked()
{
    playlist->append(MediaSource(QUrl::fromUserInput(ui->networkStreamURL->text())));
    playlist->enqueueAndPlayNext();

    ui->networkStreamURL->setText("");

    tToast* toast = new tToast(this);
    toast->setTitle("Media Stream Added");
    toast->setText("Media Stream has been added to the playlist.");
    toast->setTimeout(5000);
    connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
    toast->show(this);
}

void MainWindow::dataDecoded(QMap<AudioDataOutput::Channel, QVector<qint16>> data) {
    ui->visualisationFrame->setVisualisation(data.value(AudioDataOutput::LeftChannel));
    ui->visualisationFrame->update();
}

void MainWindow::on_visualisationFrame_visualisationRateChanged(int size)
{
    dataOut->setDataSize(size);
}

void MainWindow::on_visualisationFrame_customContextMenuRequested(const QPoint &pos)
{
    QMenu* menu = new QMenu(this);
    menu->addSection(tr("Visualisation"));
    menu->addAction(ui->actionScope);
    menu->addAction(ui->actionLines);
    menu->addAction(ui->actionCircle);
    menu->addAction(ui->actionBars);
    menu->exec(ui->visualisationFrame->mapToGlobal(pos));
}

void MainWindow::on_actionScope_triggered()
{
    ui->visualisationFrame->setVisualisationType(VisualisationFrame::Scope);
    ui->actionScope->setChecked(true);
    ui->actionLines->setChecked(false);
    ui->actionCircle->setChecked(false);
    ui->actionBars->setChecked(false);
}

void MainWindow::on_actionLines_triggered()
{
    ui->visualisationFrame->setVisualisationType(VisualisationFrame::Lines);
    ui->actionScope->setChecked(false);
    ui->actionLines->setChecked(true);
    ui->actionCircle->setChecked(false);
    ui->actionBars->setChecked(false);
}

void MainWindow::on_actionCircle_triggered()
{
    ui->visualisationFrame->setVisualisationType(VisualisationFrame::Circle);
    ui->actionScope->setChecked(false);
    ui->actionLines->setChecked(false);
    ui->actionCircle->setChecked(true);
    ui->actionBars->setChecked(false);

}

void MainWindow::on_actionAbout_triggered()
{
    AboutWindow window;
    window.exec();
}

void MainWindow::on_OpenFileButton_clicked()
{
    ui->actionOpen->trigger();
}

void MainWindow::on_library_activated(const QModelIndex &index)
{
}

void MainWindow::on_library_doubleClicked(const QModelIndex &index)
{
    MediaSource source(library->data(index, Qt::UserRole).toString());
    playlist->append(source);
    playlist->playItem(playlist->rowCount() - 1);
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    //Move items around accordingly
    if (this->height() < 400 * theLibsGlobal::getDPIScaling() || this->width() < 450 * theLibsGlobal::getDPIScaling()) {
        //Use mini player
        ui->contentFrame->setVisible(false);
        ui->musicDivider->setVisible(false);
        ui->topLine->setVisible(false);
        ui->playlistContainerMainFrame->setVisible(false);
        ui->playlistContainerUnderFrame->setVisible(true);
        ui->playlistContainerUnder->addWidget(ui->playlistWidget);
        ui->appTitleLabel->setText(tr("theBeat"));
    } else {
        ui->contentFrame->setVisible(true);
        ui->musicDivider->setVisible(true);
        ui->topLine->setVisible(true);
        ui->playlistContainerMainFrame->setVisible(true);
        ui->playlistContainerUnderFrame->setVisible(false);
        ui->playlistContainerMain->insertWidget(0, ui->playlistWidget);

        if (this->width() < 1000 * theLibsGlobal::getDPIScaling()) {
            //Collapse sidebar
            ui->sourcesList->setMaximumSize(36 * theLibsGlobal::getDPIScaling(), ui->sourcesList->maximumHeight());
            ui->appTitleLabel->setPixmap(QIcon::fromTheme("thebeat", QIcon::fromTheme(":/icons/icon.svg")).pixmap(QSize(16, 16) * theLibsGlobal::getDPIScaling()));
        } else {
            //Expand sidebar
            ui->sourcesList->setMaximumSize(300 * theLibsGlobal::getDPIScaling(), ui->sourcesList->maximumHeight());
            ui->appTitleLabel->setText(tr("theBeat"));
        }
    }

    if (ui->searchWidget->width() != 0) {
        ui->searchWidget->setFixedWidth(ui->musicLibraryHeader->width() - ui->searchButton->width() - ui->editMusicLibraryButton->width());
    }
}

void MainWindow::on_PlaylistsView_doubleClicked(const QModelIndex &index)
{
    int play = playlist->rowCount();
    playlist->appendPlaylist(ui->PlaylistsView->model()->data(index, Qt::UserRole).toString());
    playlist->playItem(play);
}

PlaylistModel* MainWindow::getPlaylist() {
    return playlist;
}

void MainWindow::on_actionBars_triggered()
{

    ui->visualisationFrame->setVisualisationType(VisualisationFrame::Bars);
    ui->actionScope->setChecked(false);
    ui->actionLines->setChecked(false);
    ui->actionCircle->setChecked(false);
    ui->actionBars->setChecked(true);
}

void MainWindow::on_actionSave_Playlist_triggered()
{
    bool ok;
    QString playlistName = QInputDialog::getText(this, tr("Save Playlist"), tr("What do you want to call the playlist?"), QLineEdit::Normal, "", &ok);
    if (ok) {
        QDir playlistDir(QDir::homePath() + "/.themedia/playlists");
        QFile playlistFile(playlistDir.absoluteFilePath(playlistName));
        if (playlistFile.exists()) {
            if (QMessageBox::warning(this, tr("Playlist already exists"), tr("The playlist called <b>%1</b> already exists. Do you want to clear it and replace it with the current one?").arg(playlistName), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {
                //Cancel and return
                return;
            }
        }

        playlistFile.open(QFile::WriteOnly);
        playlistFile.write(playlist->createPlaylist());
        playlistFile.close();
    }
}

void MainWindow::on_actionClear_Playlist_triggered()
{
    playlist->clear();
}

void MainWindow::on_actionAdd_to_existing_playlist_triggered()
{

}

void MainWindow::on_editMusicLibraryButton_clicked()
{
    ui->libStack->setCurrentIndex(2);
    libWidget->reloadLibrary();
}

void MainWindow::on_manageMusicLibrarySplashButton_clicked()
{
    ui->editMusicLibraryButton->click();
}

void MainWindow::on_shuffleButton_toggled(bool checked)
{
    playlist->setShuffle(checked);
}

void MainWindow::on_searchButton_clicked()
{
    ui->searchWidget->setVisible(true);
    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(0);
    anim->setEndValue(ui->libraryHeaderText->width());
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start();
    connect(anim, &tVariantAnimation::finished, [=] {
        anim->deleteLater();
        ui->libraryHeaderText->setVisible(false);
    });
    connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        ui->searchWidget->setFixedWidth(value.toInt());
    });
    ui->searchLineEdit->setFocus();
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (watched == ui->searchLineEdit) {
        if (event->type() == QEvent::FocusOut) {
            if (ui->searchLineEdit->text() == "") {
                ui->libraryHeaderText->setVisible(true);
                tVariantAnimation* anim = new tVariantAnimation();
                anim->setStartValue(ui->searchWidget->width());
                anim->setEndValue(0);
                anim->setDuration(500);
                anim->setEasingCurve(QEasingCurve::InCubic);
                anim->start();
                connect(anim, &tVariantAnimation::finished, [=] {
                    anim->deleteLater();
                    ui->searchWidget->setVisible(false);
                });
                connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
                    ui->searchWidget->setFixedWidth(value.toInt());
                });
            }
        }
    } else if (watched == ui->mediaLibraryInfoWidget) {
        if (event->type() == QEvent::Paint) {
            QPainter painter(ui->mediaLibraryInfoWidget);

            if (playlistBackground.isNull()) {
                QSvgRenderer renderer(QString(":/icons/coverimage.svg"));

                QRect rect;
                rect.setSize(renderer.defaultSize().scaled(ui->mediaLibraryInfoWidget->width(), ui->mediaLibraryInfoWidget->height(), Qt::KeepAspectRatioByExpanding));
                rect.setLeft(ui->mediaLibraryInfoWidget->width() / 2 - rect.width() / 2);
                rect.setTop(ui->mediaLibraryInfoWidget->height() / 2 - rect.height() / 2);

                renderer.render(&painter, rect);

                painter.setBrush(QColor(0, 0, 0, 150));
                painter.setPen(Qt::transparent);
                painter.drawRect(0, 0, ui->mediaLibraryInfoWidget->width(), ui->mediaLibraryInfoWidget->height());
            } else {
                QRect rect;
                rect.setSize(playlistBackground.size().scaled(ui->mediaLibraryInfoWidget->width(), ui->mediaLibraryInfoWidget->height(), Qt::KeepAspectRatioByExpanding));
                rect.moveLeft(ui->mediaLibraryInfoWidget->width() / 2 - rect.width() / 2);
                rect.moveTop(ui->mediaLibraryInfoWidget->height() / 2 - rect.height() / 2);

                //Blur the background
                int radius = 30;
                QGraphicsBlurEffect* blur = new QGraphicsBlurEffect;
                blur->setBlurRadius(radius);

                QGraphicsScene scene;
                QGraphicsPixmapItem item;
                item.setPixmap(QPixmap::fromImage(playlistBackground));
                item.setGraphicsEffect(blur);
                scene.addItem(&item);

                //scene.render(&painter, QRectF(), QRectF(-radius, -radius, image.width() + radius, image.height() + radius));
                scene.render(&painter, rect.adjusted(-radius, -radius, radius, radius), QRectF(-radius, -radius, playlistBackground.width() + radius, playlistBackground.height() + radius));

                painter.setBrush(QColor(0, 0, 0, 150));
                painter.setPen(Qt::transparent);
                painter.drawRect(0, 0, ui->mediaLibraryInfoWidget->width(), ui->mediaLibraryInfoWidget->height());

                QRect rightRect;
                rightRect.setSize(playlistBackground.size().scaled(0, ui->mediaLibraryInfoWidget->height(), Qt::KeepAspectRatioByExpanding));
                rightRect.moveRight(ui->mediaLibraryInfoWidget->width());
                rightRect.moveTop(ui->mediaLibraryInfoWidget->height() / 2 - rightRect.height() / 2);
                painter.drawImage(rightRect, playlistBackground.scaled(rightRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            }

        }
    }
    return false;
}

void MainWindow::on_searchLineEdit_textChanged(const QString &arg1)
{
    library->search(arg1);
}

void MainWindow::activate() {
#ifdef Q_OS_LINUX
    XEvent event;

    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    event.xclient.message_type = XInternAtom(QX11Info::display(), "_NET_ACTIVE_WINDOW", False);
    event.xclient.window = this->winId();
    event.xclient.format = 32;

    XSendEvent(QX11Info::display(), DefaultRootWindow(QX11Info::display()), False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
    XMapRaised(QX11Info::display(), this->winId());
#endif
}

void MainWindow::on_tracksButton_toggled(bool checked)
{
    if (checked) {
        ui->libraryStack->setCurrentIndex(0);

        library->clearFilter();
        ui->libraryTrackNumber->setText(tr("%n tracks", nullptr, library->rowCount()));
        ui->listPlayOptions->setVisible(true);
        ui->libraryBackButton->setVisible(false);
        ui->libraryListTitle->setText(tr("All Tracks"));
        playlistBackground = QImage();
        ui->mediaLibraryInfoWidget->update();
    }
}

void MainWindow::on_artistsButton_toggled(bool checked)
{
    if (checked) {
        ui->libraryStack->setCurrentIndex(1);

        library->clearFilter();
        ui->libraryTrackNumber->setText(tr("%n artists", nullptr, ui->artistsView->model()->rowCount()));
        ui->listPlayOptions->setVisible(false);
        ui->libraryBackButton->setVisible(false);
        ui->libraryListTitle->setText(tr("All Artists"));
        playlistBackground = QImage();
        ui->mediaLibraryInfoWidget->update();
    }
}

void MainWindow::on_albumsButton_toggled(bool checked)
{
    if (checked) {
        ui->libraryStack->setCurrentIndex(2);

        library->clearFilter();
        ui->libraryTrackNumber->setText(tr("%n albums", nullptr, ui->albumsView->model()->rowCount()));
        ui->listPlayOptions->setVisible(false);
        ui->libraryBackButton->setVisible(false);
        ui->libraryListTitle->setText(tr("All Albums"));
        playlistBackground = QImage();
        ui->mediaLibraryInfoWidget->update();
    }
}

void MainWindow::on_artistsView_activated(const QModelIndex &index)
{
    QString filter = index.data().toString();
    ui->libraryStack->setCurrentIndex(0);
    library->filter(filter, LibraryModel::Artist);
    ui->libraryListTitle->setText(tr("Music by %1").arg(filter));

    ui->libraryTrackNumber->setText(tr("%n tracks", nullptr, library->rowCount()));
    ui->listPlayOptions->setVisible(true);
    ui->libraryBackButton->setVisible(true);
    ui->libraryBackButton->setProperty("backAction", "artist");
    playlistBackground = QImage();
    ui->mediaLibraryInfoWidget->update();
}

void MainWindow::on_libraryBackButton_clicked()
{
    if (ui->libraryBackButton->property("backAction").toString() == "artist") {
        on_artistsButton_toggled(true);
    } else if (ui->libraryBackButton->property("backAction").toString() == "album") {
        on_albumsButton_toggled(true);
    }
}

void MainWindow::on_enqueueAllButton_clicked()
{
    for (int i = 0; i < library->rowCount(); i++) {
        MediaSource source(library->data(library->index(i, 0), Qt::UserRole).toString());
        playlist->append(source);
    }

    if (playlist->currentItem() == -1) {
        playlist->enqueueNext();
        playlist->playItem(0, true);
    }
}

void MainWindow::on_playAllButton_clicked()
{
    //Clear the playlist and then enqueue everything
    ui->actionClear_Playlist->trigger();
    ui->enqueueAllButton->click();
}

void MainWindow::on_albumsView_activated(const QModelIndex &index)
{
    QString filter = index.data().toString();
    ui->libraryStack->setCurrentIndex(0);
    library->filter(filter, LibraryModel::Album);
    ui->libraryListTitle->setText(tr("Music in %1").arg(filter));

    ui->libraryTrackNumber->setText(tr("%n tracks", nullptr, library->rowCount()));
    ui->listPlayOptions->setVisible(true);
    ui->libraryBackButton->setVisible(true);
    ui->libraryBackButton->setProperty("backAction", "album");

    QModelIndex libIndex = library->index(0, 0);
    if (libIndex.isValid()) {
        setLibraryCoverImage(TagCache::getAlbumArt(libIndex.data(Qt::UserRole).toString()));
    } else {
        playlistBackground = QImage();
        ui->mediaLibraryInfoWidget->update();
    }
}

void MainWindow::on_shuffleAllButton_clicked()
{
    //Clear the playlist, turn on shuffling and then enqueue everything
    ui->actionClear_Playlist->trigger();
    ui->shuffleButton->setChecked(true);
    ui->enqueueAllButton->click();
}


void MainWindow::on_sourcesStack_currentChanged(int arg1)
{
    ui->sourcesList->setCurrentRow(arg1);
}

void MainWindow::setLibraryCoverImage(QImage image) {
    playlistBackground = image;
    ui->mediaLibraryInfoWidget->update();
}
