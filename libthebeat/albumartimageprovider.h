#ifndef ALBUMARTIMAGEPROVIDER_H
#define ALBUMARTIMAGEPROVIDER_H

#include <QQuickAsyncImageProvider>
#include <libthebeat_global.h>

class LIBTHEBEAT_EXPORT AlbumArtImageProvider : public QQuickAsyncImageProvider {
    public:
        explicit AlbumArtImageProvider();

        // QQuickAsyncImageProvider interface
    public:
        QQuickImageResponse* requestImageResponse(const QString& id, const QSize& requestedSize);
};

struct AlbumArtImageProviderResponsePrivate;
class AlbumArtImageProviderResponse : public QQuickImageResponse {
        Q_OBJECT

    public:
        AlbumArtImageProviderResponse(const QString& id, const QSize& requestedSize);
        ~AlbumArtImageProviderResponse();

    private:
        AlbumArtImageProviderResponsePrivate* d;

        void checkForImage();

        // QQuickImageResponse interface
    public:
        QQuickTextureFactory* textureFactory() const;
};

#endif // ALBUMARTIMAGEPROVIDER_H
