#include "gsturlhandler.h"

#include "gstcdplayback.h"
#include <QUrl>

GstUrlHandler::GstUrlHandler(QObject* parent) :
    UrlHandler{parent} {
}

MediaItem* GstUrlHandler::itemForUrl(QUrl url) {
    if (url.scheme() == "gst") {
        return new GstCdPlayback("sr1", 2);
    }
    return nullptr;
}
