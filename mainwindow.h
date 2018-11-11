#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <phonon/MediaSource>
#include <phonon/MediaObject>
#include <phonon/MediaController>
#include <phonon/AudioDataOutput>
#include <phonon/AudioOutput>
#include <QDateTime>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QListView>
#include <QListWidget>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QAction>
#include <QStackedWidget>
#include <QLineEdit>
#include <ttoast.h>
#include <QMenu>
#include <QSettings>
#include <QTableWidget>
#include <QResizeEvent>
#include <tnotification.h>
#include <QToolButton>
#include <QTreeView>
#include <QHeaderView>
#include "visualisationframe.h"
#include "playlistmodel.h"
#include "dbusadaptors.h"
#include "aboutwindow.h"
#include "librarymodel.h"
#include "librarymanagewidget.h"

#ifdef BUILD_DISCORD
#include "discordintegration.h"
#endif

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
    PlaylistModel* getPlaylist();

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

    void on_repeatButton_toggled(bool checked);

    void on_sourcesList_currentRowChanged(int currentRow);

    void on_AddNetworkStreamButton_clicked();

    void dataDecoded(QMap<Phonon::AudioDataOutput::Channel,QVector<qint16>> data);

    void on_visualisationFrame_visualisationRateChanged(int size);

    void on_visualisationFrame_customContextMenuRequested(const QPoint &pos);

    void on_actionScope_triggered();

    void on_actionLines_triggered();

    void on_actionCircle_triggered();

    void on_actionAbout_triggered();

    void on_OpenFileButton_clicked();

    void on_library_activated(const QModelIndex &index);

    void on_library_doubleClicked(const QModelIndex &index);

    void on_PlaylistsView_doubleClicked(const QModelIndex &index);

    void on_actionBars_triggered();

    void on_actionSave_Playlist_triggered();

    void on_actionClear_Playlist_triggered();

    void on_actionAdd_to_existing_playlist_triggered();

    void on_editMusicLibraryButton_clicked();

    void on_manageMusicLibrarySplashButton_clicked();

    void on_shuffleButton_toggled(bool checked);

    void on_searchButton_clicked();

    void on_searchLineEdit_textChanged(const QString &arg1);

    void activate();

    void on_tracksButton_toggled(bool checked);

    void on_artistsButton_toggled(bool checked);

    void on_albumsButton_toggled(bool checked);

    void on_artistsView_activated(const QModelIndex &index);

    void on_libraryBackButton_clicked();

    void on_enqueueAllButton_clicked();

    void on_playAllButton_clicked();

    void on_albumsView_activated(const QModelIndex &index);

    private:
    Ui::MainWindow *ui;

    bool eventFilter(QObject* watched, QEvent* event);

    PlaylistModel* playlist;
    LibraryModel* library;

    MediaObject *player, *cdFinder;
    MediaController *controller, *cdController;
    AudioDataOutput* dataOut;

    LibraryManageWidget* libWidget;

#ifdef BUILD_DISCORD
    DiscordIntegration* discord;
#endif

    QVariantMap mprisMetadataMap;
    void resizeEvent(QResizeEvent* event);
};

#endif // MAINWINDOW_H
