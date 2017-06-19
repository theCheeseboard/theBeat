#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->currentMediaFrame->setVisible(false);

    player = new MediaObject(this);
    playlist = new PlaylistModel(player);

    createPath(player, new AudioOutput(Phonon::MusicCategory, this));
    connect(player, SIGNAL(metaDataChanged()), this, SLOT(updateMetadata()));
    //connect(player, SIGNAL(aboutToFinish()), this, SLOT(playerAboutToFinish()));
    connect(player, SIGNAL(aboutToFinish()), playlist, SLOT(enqueueNext()));
    connect(player, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(playerStateChanged(Phonon::State,Phonon::State)));

    ui->seeker->setMediaObject(player);

    ui->playlistWidget->setModel(playlist);

    ui->sourcesList->addItem("Music Library");

    QListWidgetItem* openFileItem = new QListWidgetItem("Open File");
    ui->sourcesList->addItem(openFileItem);

    QListWidgetItem* openUrlItem = new QListWidgetItem("Open Network Stream");
    ui->sourcesList->addItem(openUrlItem);

    new MediaPlayer2Adaptor(this);
    new PlayerAdaptor(this);

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/org/mpris/MediaPlayer2", this, QDBusConnection::ExportAdaptors);
    dbus.registerService("org.mpris.MediaPlayer2.theBeat");
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
    QStringList Title = player->metaData(Phonon::TitleMetaData);

    QStringList metadata;
    if (Title.count() > 0) {
        if (Title.at(0) != ui->currentTitleLabel->text()) {
            ui->currentTitleLabel->setText(Title.at(0));
        }
        mprisMetadataMap.insert("xesam:title", Title.first());
    } else {
        mprisMetadataMap.remove("xesam:title");
    }

    QStringList Artist = player->metaData(Phonon::ArtistMetaData);
    if (Artist.count() > 0) {
        metadata.append(Artist.at(0));
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
    //playlist->enqueueNext();

}

void MainWindow::on_backButton_clicked()
{

}

void MainWindow::playerAboutToFinish() {
    /*if (currentPlaylist + 1 == playlist.count()) {
        currentPlaylist = 0;
    } else {
        currentPlaylist++;
    }

    player->enqueue(playlist.at(currentPlaylist));*/
}

void MainWindow::playerStateChanged(State newState, State oldState) {
    if (newState == PlayingState) {
        ui->playButton->setIcon(QIcon::fromTheme("media-playback-pause"));
    } else {
        ui->playButton->setIcon(QIcon::fromTheme("media-playback-start"));
    }
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
    switch (index.row()) {
        case 1: //Open File
            ui->actionOpen->trigger();
    }
}
