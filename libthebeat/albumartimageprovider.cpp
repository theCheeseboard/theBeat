#include "albumartimageprovider.h"
#include "playlist.h"
#include "statemanager.h"
#include <QTimer>
#include <QUuid>

AlbumArtImageProvider::AlbumArtImageProvider() :
    QQuickAsyncImageProvider{} {
}

QQuickImageResponse* AlbumArtImageProvider::requestImageResponse(const QString& id, const QSize& requestedSize) {
    return new AlbumArtImageProviderResponse(id, requestedSize);
}

struct AlbumArtImageProviderResponsePrivate {
        QString id;
        QSize requestedSize;

        MediaItem* item = nullptr;
        QImage loadedAlbumArt;
};

AlbumArtImageProviderResponse::AlbumArtImageProviderResponse(const QString& id, const QSize& requestedSize) :
    d{new AlbumArtImageProviderResponsePrivate()} {
    d->id = id;
    d->requestedSize = requestedSize;

    auto uuid = QUuid::fromString(d->id);
    for (auto item : StateManager::instance()->playlist()->items()) {
        if (item->uuid() == uuid) {
            d->item = item;
            break;
        }
    }

    if (d->item) {
        connect(d->item, &MediaItem::metadataChanged, this, &AlbumArtImageProviderResponse::checkForImage);
        connect(d->item, &MediaItem::destroyed, this, [this] {
            d->item = nullptr;
            emit finished();
        });
    }

    QTimer::singleShot(0, this, &AlbumArtImageProviderResponse::checkForImage);
}

AlbumArtImageProviderResponse::~AlbumArtImageProviderResponse() {
    delete d;
}

void AlbumArtImageProviderResponse::checkForImage() {
    if (!d->loadedAlbumArt.isNull()) return;
    if (!d->item) {
        emit finished();
        return;
    }

    // Try to load in the album art and try again later if it is null
    d->loadedAlbumArt = d->item->albumArt();
    if (d->loadedAlbumArt.isNull()) return;
    emit finished();
}

QQuickTextureFactory* AlbumArtImageProviderResponse::textureFactory() const {
    return QQuickTextureFactory::textureFactoryForImage(d->loadedAlbumArt);
}
