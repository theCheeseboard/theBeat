#ifndef GSTURLHANDLER_H
#define GSTURLHANDLER_H

#include <urlhandler.h>
#include <QObject>

class GstUrlHandler : public UrlHandler
{
    Q_OBJECT
public:
    explicit GstUrlHandler(QObject *parent = nullptr);

    // UrlHandler interface
public:
    MediaItem *itemForUrl(QUrl url);
};

#endif // GSTURLHANDLER_H
