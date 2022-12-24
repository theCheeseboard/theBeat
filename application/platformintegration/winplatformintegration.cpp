#include "winplatformintegration.h"

#include <QSysInfo>
#include <QLocale>
#include <QBuffer>
#include <QUrl>
#include <QOperatingSystemVersion>
#include <robuffer.h>

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
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;

namespace abi {
    using namespace ABI::Windows::Media;
}

class QByteArrayBackedIBuffer : public winrt::implements<QByteArrayBackedIBuffer, IBuffer, Windows::Storage::Streams::IBufferByteAccess> {
        QByteArray buf;
        uint32_t length{};

    public:
        QByteArrayBackedIBuffer(QByteArray buffer) {
            this->buf = buffer;
        }

        uint32_t Capacity() const {
            return buf.length();
        }

        uint32_t Length() const {
            return length;
        }

        void Length(uint32_t value) {
            if (value > buf.length()) {
                throw winrt::hresult_invalid_argument();
            }

            length = value;
        }

        HRESULT __stdcall Buffer(uint8_t** value) {
            *value = reinterpret_cast<uint8_t*>(buf.data());
            return S_OK;
        }

};

struct WinPlatformIntegrationPrivate {
    MediaItem* currentItem{ };
    QWidget* parentWindow{ };

    SystemMediaTransportControls smtc{ nullptr };

    bool metadataUpdateRequired = true;
    winrt::fire_and_forget updateSMTC();
};

WinPlatformIntegration::WinPlatformIntegration(QWidget* parent) : QObject(parent) {
    d = new WinPlatformIntegrationPrivate();
    d->parentWindow = parent;

    IActivationFactory factory = winrt::get_activation_factory<SystemMediaTransportControls>();
    winrt::com_ptr<ISystemMediaTransportControlsInterop> interop = factory.as<ISystemMediaTransportControlsInterop>();

    interop->GetForWindow(reinterpret_cast<HWND>(d->parentWindow->winId()), winrt::guid_of<abi::ISystemMediaTransportControls>(), winrt::put_abi(d->smtc));

    d->smtc.ButtonPressed([ = ](SystemMediaTransportControls smtc, SystemMediaTransportControlsButtonPressedEventArgs e) {
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
        connect(d->currentItem, &MediaItem::metadataChanged, this, [ = ] {
            d->metadataUpdateRequired = true;
            updateSMTC();
        });
        connect(d->currentItem, &MediaItem::elapsedChanged, this, &WinPlatformIntegration::updateSMTC);
        connect(d->currentItem, &MediaItem::durationChanged, this, &WinPlatformIntegration::updateSMTC);

        d->metadataUpdateRequired = true;
        updateSMTC();
    }
}

void WinPlatformIntegration::updateSMTC() {
    //Forward this call to the private implementation
    d->updateSMTC();
}

#include <QDebug>
winrt::fire_and_forget WinPlatformIntegrationPrivate::updateSMTC() {
    Playlist* playlist = StateManager::instance()->playlist();

    smtc.IsPlayEnabled(true);
    smtc.IsPauseEnabled(true);
    smtc.IsNextEnabled(true);
    smtc.IsPreviousEnabled(true);
    smtc.ShuffleEnabled(playlist->shuffle());
    smtc.AutoRepeatMode(playlist->repeatOne() ? MediaPlaybackAutoRepeatMode::Track : MediaPlaybackAutoRepeatMode::None);

    switch (playlist->state()) {
        case Playlist::Playing:
            smtc.PlaybackStatus(MediaPlaybackStatus::Playing);
            break;
        case Playlist::Paused:
            smtc.PlaybackStatus(MediaPlaybackStatus::Paused);
            break;
        case Playlist::Stopped:
            smtc.PlaybackStatus(MediaPlaybackStatus::Stopped);
            break;
    }

    if (metadataUpdateRequired) {
        auto updater = smtc.DisplayUpdater();
        updater.Type(MediaPlaybackType::Music);
        updater.MusicProperties().Title(currentItem->title().toStdWString().c_str());
        updater.MusicProperties().Artist(QLocale().createSeparatedList(currentItem->authors()).toStdWString().c_str());
        updater.MusicProperties().AlbumTitle(currentItem->album().toStdWString().c_str());

//        QByteArray albumArt;
//        QBuffer albumArtBuffer(&albumArt);
//        albumArtBuffer.open(QBuffer::WriteOnly);
//        currentItem->albumArt().save(&albumArtBuffer, "PNG");
//        albumArtBuffer.close();

//        if (!albumArt.isNull()) {
//            InMemoryRandomAccessStream memoryStream;
//            co_await memoryStream.WriteAsync(winrt::make<QByteArrayBackedIBuffer>(albumArt));
//            memoryStream.Seek(0);

//            auto randomAccessStream = RandomAccessStreamReference::CreateFromStream(memoryStream);
//            updater.Thumbnail(randomAccessStream);
//        }

        updater.Update();
    }

    SystemMediaTransportControlsTimelineProperties timeline;
    timeline.StartTime(0ms);
    timeline.MinSeekTime(timeline.StartTime());
    timeline.Position(std::chrono::milliseconds(currentItem->elapsed()));
    timeline.EndTime(std::chrono::milliseconds(currentItem->duration()));
    timeline.MaxSeekTime(timeline.EndTime());
    smtc.UpdateTimelineProperties(timeline);
    co_return;
}
