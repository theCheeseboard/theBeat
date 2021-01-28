#ifndef AVFOUNDATIONURLHANDLER_H
#define AVFOUNDATIONURLHANDLER_H

#include <urlhandler.h>

class AvFoundationUrlHandler : public UrlHandler
{
    Q_OBJECT
public:
    explicit AvFoundationUrlHandler(QObject *parent = nullptr);

signals:


    // UrlHandler interface
public:
    MediaItem *itemForUrl(QUrl url);
};

#endif // AVFOUNDATIONURLHANDLER_H
