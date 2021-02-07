#include "avfoundationurlhandler.h"

#include "avfoundationmediaitem.h"
#include <QUrl>

AvFoundationUrlHandler::AvFoundationUrlHandler(QObject* parent) : UrlHandler(parent) {

}


MediaItem* AvFoundationUrlHandler::itemForUrl(QUrl url) {
    if (url.scheme() == "file") {
        return new AvFoundationMediaItem(url);
    }
    return nullptr;
}
