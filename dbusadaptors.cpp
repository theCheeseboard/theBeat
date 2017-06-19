#include "dbusadaptors.h"

MediaPlayer2Adaptor::MediaPlayer2Adaptor(MainWindow *parent) : QDBusAbstractAdaptor(parent) {

}

MediaPlayer2Adaptor::~MediaPlayer2Adaptor() {

}

PlayerAdaptor::PlayerAdaptor(MainWindow *parent) : QDBusAbstractAdaptor(parent) {
    mainWindow = parent;
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

void PlayerAdaptor::Next() {

}

void PlayerAdaptor::Previous() {

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
    mainWindow->getPlayer()->seek(position);
}
