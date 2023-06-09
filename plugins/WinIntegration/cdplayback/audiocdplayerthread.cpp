#include "audiocdplayerthread.h"

#include <statemanager.h>
#include <playlist.h>
#include <tlogger.h>
#include <Windows.h>
#include <winrt/Windows.Foundation.Collections.h>

struct AudioCdPlayerThreadPrivate {
    QThread thread;
    winrt::CDLib::IAudioCDPlayer audioCdPlayer;
};

AudioCdPlayerThread* AudioCdPlayerThread::instance() {
    static AudioCdPlayerThread* instance = new AudioCdPlayerThread();

    QMetaObject::Connection* connection = new QMetaObject::Connection();
    *connection = connect(instance, &AudioCdPlayerThread::ready, [ = ] {
        disconnect(*connection);
        delete connection;

        connect(StateManager::instance()->playlist(), &Playlist::volumeChanged, [ = ](double volume) {
            instance->player().Volume(volume);
        });
        instance->player().Volume(StateManager::instance()->playlist()->volume());
    });

    return instance;
}

winrt::CDLib::IAudioCDPlayer AudioCdPlayerThread::player() {
    return d->audioCdPlayer;
}

void AudioCdPlayerThread::start() {
    tDebug("AudioCdPlayerThread") << "Started";
    this->run();
}

AudioCdPlayerThread::AudioCdPlayerThread(QObject* parent) : QObject(parent) {
    d = new AudioCdPlayerThreadPrivate();
    d->thread.start();
    this->moveToThread(&d->thread);
}

void AudioCdPlayerThread::run() {
    try {
        winrt::init_apartment(winrt::apartment_type::single_threaded);
        d->audioCdPlayer = winrt::CDLib::AudioCDPlayer::GetPlayer();
    } catch (winrt::hresult_error err) {
        tWarn("AudioCdPlayerThread") << "Error creating player:" << QString::fromWCharArray(err.message().c_str());
        return;
    }

    emit ready();

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
