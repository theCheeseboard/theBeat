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

using namespace std::chrono_literals;
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
        Playlist* playlist = StateManager::instance()->playlist();
        switch (e.Button()) {
            case SystemMediaTransportControlsButton::Play:
                playlist->metaObject()->invokeMethod(playlist, "play", Qt::QueuedConnection);
                break;
            case SystemMediaTransportControlsButton::Pause:
                playlist->metaObject()->invokeMethod(playlist, "pause", Qt::QueuedConnection);
                break;
            case SystemMediaTransportControlsButton::Next:
                playlist->metaObject()->invokeMethod(playlist, "next", Qt::QueuedConnection);
                break;
            case SystemMediaTransportControlsButton::Previous:
                playlist->metaObject()->invokeMethod(playlist, "previous", Qt::QueuedConnection);
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
    connect(StateManager::instance()->playlist(), &Playlist::shuffleChanged, this, &WinPlatformIntegration::updateCurrentItem);
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

    Playlist* playlist = StateManager::instance()->playlist();

    d->smtc.IsPlayEnabled(true);
    d->smtc.IsPauseEnabled(true);
    d->smtc.IsNextEnabled(true);
    d->smtc.IsPreviousEnabled(true);
    d->smtc.ShuffleEnabled(playlist->shuffle());
    d->smtc.AutoRepeatMode(playlist->repeatOne() ? MediaPlaybackAutoRepeatMode::Track : MediaPlaybackAutoRepeatMode::None);

    switch (playlist->state()) {
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

    SystemMediaTransportControlsTimelineProperties timeline;
    timeline.StartTime(0ms);
    timeline.MinSeekTime(timeline.StartTime());
    timeline.Position(std::chrono::milliseconds(d->currentItem->elapsed()));
    timeline.EndTime(std::chrono::milliseconds(d->currentItem->duration()));
    timeline.MaxSeekTime(timeline.EndTime());
    d->smtc.UpdateTimelineProperties(timeline);

    updater.Update();
}
