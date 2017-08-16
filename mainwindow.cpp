#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->currentMediaFrame->setVisible(false);

    player = new MediaObject(this);
    dataOut = new AudioDataOutput(this);
    playlist = new PlaylistModel(player);

    createPath(player, new AudioOutput(Phonon::MusicCategory, this));
    connect(player, SIGNAL(metaDataChanged()), this, SLOT(updateMetadata()));
    //connect(player, SIGNAL(aboutToFinish()), this, SLOT(playerAboutToFinish()));
    connect(player, SIGNAL(aboutToFinish()), playlist, SLOT(enqueueNext()));
    connect(player, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(playerStateChanged(Phonon::State,Phonon::State)));
    connect(player, SIGNAL(currentSourceChanged(Phonon::MediaSource)), this, SLOT(updateMetadata()));
    connect(player, &MediaObject::totalTimeChanged, [=](qint64 totalTime) {
        mprisMetadataMap.insert("mpris:length", totalTime * 1000);
    });

    dataOut->setDataSize(ui->visualisationFrame->width());
    connect(dataOut, SIGNAL(dataReady(QMap<Phonon::AudioDataOutput::Channel,QVector<qint16>>)), this, SLOT(dataDecoded(QMap<Phonon::AudioDataOutput::Channel,QVector<qint16>>)));
    createPath(player, dataOut);

    ui->seeker->setMediaObject(player);

    ui->playlistWidget->setModel(playlist);

    QMenu* playlistMenu = new QMenu();
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
    ui->library->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    ui->PlaylistsView->setModel(new PlaylistFileModel);
}

MainWindow::~MainWindow()
{
    delete ui;
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
        ui->currentTitleLabel->setText("");
        mprisMetadataMap.remove("xesam:title");
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
    ui->albumArtLabel->setPixmap(QIcon::fromTheme("audio").pixmap(32, 32));

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
    if (newState == PlayingState) {
        ui->playButton->setIcon(QIcon::fromTheme("media-playback-pause"));
    } else {
        ui->playButton->setIcon(QIcon::fromTheme("media-playback-start"));
    }

    QString status;
    if (newState == PlayingState) {
        status = "Playing";
    } else if (newState == PausedState) {
        status = "Paused";
    } else {
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
    menu->addSection("Visualisation");
    menu->addAction(ui->actionScope);
    menu->addAction(ui->actionLines);
    menu->addAction(ui->actionCircle);
    menu->exec(ui->visualisationFrame->mapToGlobal(pos));
}

void MainWindow::on_actionScope_triggered()
{
    ui->visualisationFrame->setVisualisationType(VisualisationFrame::Scope);
    ui->actionScope->setChecked(true);
    ui->actionLines->setChecked(false);
    ui->actionCircle->setChecked(false);
}

void MainWindow::on_actionLines_triggered()
{
    ui->visualisationFrame->setVisualisationType(VisualisationFrame::Lines);
    ui->actionScope->setChecked(false);
    ui->actionLines->setChecked(true);
    ui->actionCircle->setChecked(false);
}

void MainWindow::on_actionCircle_triggered()
{
    ui->visualisationFrame->setVisualisationType(VisualisationFrame::Circle);
    ui->actionScope->setChecked(false);
    ui->actionLines->setChecked(false);
    ui->actionCircle->setChecked(true);

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

void MainWindow::on_actionManage_Library_triggered()
{
    LibraryManageDialog d;
    d.exec();
    library->reloadData();
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
    if (this->height() < 400 || this->width() < 450) {
        ui->contentFrame->setVisible(false);
        ui->musicDivider->setVisible(false);
        ui->playlistContainerMainFrame->setVisible(false);
        ui->playlistContainerUnderFrame->setVisible(true);
        ui->playlistContainerUnder->addWidget(ui->playlistWidget);
    } else {
        ui->contentFrame->setVisible(true);
        ui->musicDivider->setVisible(true);
        ui->playlistContainerMainFrame->setVisible(true);
        ui->playlistContainerUnderFrame->setVisible(false);
        ui->playlistContainerMain->insertWidget(0, ui->playlistWidget);

        if (this->width() < 1000) {
            //ui->sourcesList->setVisible(false);
            ui->sourcesList->setMaximumSize(36, ui->sourcesList->maximumHeight());
            //ui->sourcesDivider->setVisible(false);
        } else {
            //ui->sourcesList->setVisible(true);
            ui->sourcesList->setMaximumSize(300, ui->sourcesList->maximumHeight());
            //ui->sourcesDivider->setVisible(true);
        }
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
