#include "winplatformintegration.h"

#include <QSysInfo>
#include <QLocale>

#pragma comment(lib, "windowsapp")

#include <statemanager.h>
#include <playlist.h>

#include <windows.media.h>
#include <systemmediatransportcontrolsinterop.h>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Media;

namespace abi
{
    using namespace ABI::Windows::Media;
}

struct WinPlatformIntegrationPrivate {
    MediaItem* currentItem = nullptr;
    QWidget* parentWindow;

    SystemMediaTransportControls smtc{ nullptr };
};

WinPlatformIntegration::WinPlatformIntegration(QWidget *parent) : QObject(parent) {
    d = new WinPlatformIntegrationPrivate();
    d->parentWindow = parent;

    //TODO: Add a version check for Windows 8 and later

    //Initialise the SMTC
    IActivationFactory factory = winrt::get_activation_factory<SystemMediaTransportControls>();
    winrt::com_ptr<ISystemMediaTransportControlsInterop> interop = factory.as<ISystemMediaTransportControlsInterop>();

    interop->GetForWindow(reinterpret_cast<HWND>(d->parentWindow->winId()), winrt::guid_of<abi::ISystemMediaTransportControls>(), winrt::put_abi(d->smtc));

    d->smtc.ButtonPressed([=](SystemMediaTransportControls smtc, SystemMediaTransportControlsButtonPressedEventArgs e) {
        switch (e.Button()) {
            case SystemMediaTransportControlsButton::Play:
                StateManager::instance()->playlist()->play();
                break;
            case SystemMediaTransportControlsButton::Pause:
                StateManager::instance()->playlist()->pause();
                break;
            case SystemMediaTransportControlsButton::Next:
                StateManager::instance()->playlist()->next();
                break;
            case SystemMediaTransportControlsButton::Previous:
                StateManager::instance()->playlist()->previous();
                break;
            case SystemMediaTransportControlsButton::Stop:
            case SystemMediaTransportControlsButton::Record:
            case SystemMediaTransportControlsButton::FastForward:
            case SystemMediaTransportControlsButton::Rewind:
            case SystemMediaTransportControlsButton::ChannelUp:
            case SystemMediaTransportControlsButton::ChannelDown:
                break;
        }
    });

    connect(StateManager::instance()->playlist(), &Playlist::currentItemChanged, this, &WinPlatformIntegration::updateCurrentItem);
    updateCurrentItem();
}

WinPlatformIntegration::~WinPlatformIntegration() {
    delete d;
}

void WinPlatformIntegration::updateCurrentItem() {
    if (d->currentItem) {
        d->currentItem->disconnect(this);
    }
    d->currentItem = StateManager::instance()->playlist()->currentItem();
    if (d->currentItem) {
        connect(d->currentItem, &MediaItem::metadataChanged, this, &WinPlatformIntegration::updateSMTC);
        connect(d->currentItem, &MediaItem::elapsedChanged, this, &WinPlatformIntegration::updateSMTC);
        connect(d->currentItem, &MediaItem::durationChanged, this, &WinPlatformIntegration::updateSMTC);
        updateSMTC();
    }
}

void WinPlatformIntegration::updateSMTC() {
    //TODO: Add a version check for Windows 8 and later

    d->smtc.IsPlayEnabled(true);
    d->smtc.IsPauseEnabled(true);
    d->smtc.IsNextEnabled(true);
    d->smtc.IsPreviousEnabled(true);

    switch (StateManager::instance()->playlist()->state()) {
        case Playlist::Playing:
            d->smtc.PlaybackStatus(MediaPlaybackStatus::Playing);
            break;
        case Playlist::Paused:
            d->smtc.PlaybackStatus(MediaPlaybackStatus::Paused);
            break;
        case Playlist::Stopped:
            d->smtc.PlaybackStatus(MediaPlaybackStatus::Stopped);
            break;
    }

    auto updater = d->smtc.DisplayUpdater();
    updater.Type(MediaPlaybackType::Music);
    updater.MusicProperties().Title(d->currentItem->title().toStdWString().c_str());
    updater.MusicProperties().Artist(QLocale().createSeparatedList(d->currentItem->authors()).toStdWString().c_str());
    updater.MusicProperties().AlbumTitle(d->currentItem->album().toStdWString().c_str());

    // WinRT supports async/await heavily. There is no sync API for getting a StorageFile from a path
    // C++20 has support for coroutines (async/await) using co_await. C++/WinRT uses this functionality
    // This method returns fire_and_forget, allowing the coroutine to be generated - presumably, you
    // do not need to await this function at the call site
//    StorageFile file = co_await StorageFile::GetFileFromPathAsync(LR"(C:\Users\reflectronic\Desktop\Evening_1024.jpg)");
//    updater.Thumbnail(RandomAccessStreamReference::CreateFromFile(file));

    updater.Update();
}
