#include "dbusadaptors.h"

MediaPlayer2Adaptor::MediaPlayer2Adaptor(MainWindow *parent) : QDBusAbstractAdaptor(parent) {

}

MediaPlayer2Adaptor::~MediaPlayer2Adaptor() {

}

void MediaPlayer2Adaptor::Raise() {
   qobject_cast<MainWindow*>(parent())->raise();
}

PlayerAdaptor::PlayerAdaptor(MainWindow *parent) : QDBusAbstractAdaptor(parent) {
    mainWindow = parent;
    connect(mainWindow->getPlayer(), &MediaObject::tick, [=](qint64 time) {
        emit Seeked(time * 1000);
    });
}

PlayerAdaptor::~PlayerAdaptor() {

}

QVariantMap PlayerAdaptor::metadata() const {
    return mainWindow->metadataMap();
}

QString PlayerAdaptor::playbackStatus() const {
    Phonon::State s = mainWindow->getPlayer()->state();
    if (s == Phonon::PlayingState) {
        return "Playing";
    } else if (s == Phonon::PausedState) {
        return "Paused";
    } else {
        return "Stopped";
    }
}

qlonglong PlayerAdaptor::position() const {
    return mainWindow->getPlayer()->currentTime();
}

void PlayerAdaptor::Seek(qint64 position) {
    qint64 seek = mainWindow->getPlayer()->currentTime() + position / 1000;
    if (seek < 0) {
        seek = 0;
    } else if (seek > mainWindow->getPlayer()->totalTime()) {
        Next();
        return;
    }
    mainWindow->getPlayer()->seek(seek);
}

void PlayerAdaptor::Next() {
    mainWindow->getPlaylist()->playNext();
}

void PlayerAdaptor::Previous() {
    mainWindow->getPlaylist()->skipBack();
}

void PlayerAdaptor::PlayPause() {
    if (playbackStatus() == "Playing") {
        Pause();
    } else {
        Play();
    }
}

void PlayerAdaptor::Pause() {
    mainWindow->getPlayer()->pause();
}

void PlayerAdaptor::Play() {
    mainWindow->getPlayer()->play();
}

void PlayerAdaptor::Stop() {
    mainWindow->getPlayer()->pause();
}

void PlayerAdaptor::SetPosition(QDBusObjectPath track, qlonglong position) {
    mainWindow->getPlayer()->seek(position / 1000);
}
