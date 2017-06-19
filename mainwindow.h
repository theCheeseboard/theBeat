#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <phonon/MediaSource>
#include <phonon/MediaObject>
#include <phonon/MediaController>
#include <phonon/AudioDataOutput>
#include <phonon/AudioOutput>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QListView>
#include <QListWidget>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QAction>
#include "visualisationframe.h"
#include "playlistmodel.h"
#include "dbusadaptors.h"

using namespace Phonon;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QVariantMap metadataMap();

    MediaObject* getPlayer();

private slots:
    void on_actionOpen_triggered();

    void updateMetadata();

    void on_playButton_clicked();

    void on_nextButton_clicked();

    void on_backButton_clicked();

    void playerAboutToFinish();

    void playerStateChanged(Phonon::State newState, Phonon::State oldState);

    void on_playlistWidget_activated(const QModelIndex &index);

    void on_sourcesList_activated(const QModelIndex &index);

    void on_actionExit_triggered();

private:
    Ui::MainWindow *ui;

    PlaylistModel* playlist;

    MediaObject* player;
    MediaController* controller;
    AudioDataOutput* dataOut;

    QVariantMap mprisMetadataMap;
};

#endif // MAINWINDOW_H
