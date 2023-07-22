#ifndef IURLMANAGER_H
#define IURLMANAGER_H

#include <QUrl>

class MediaItem;
class UrlHandler;

class IUrlManager {
        Q_GADGET

    public:
        virtual MediaItem* itemForUrl(QUrl url) = 0;
        virtual void registerHandler(UrlHandler* handler) = 0;
};

#endif // IURLMANAGER_H
