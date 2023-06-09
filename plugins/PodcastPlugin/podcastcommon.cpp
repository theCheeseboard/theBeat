#include "podcastcommon.h"

#include <QCoroNetwork>
#include <QFile>
#include <QNetworkAccessManager>
#include <texception.h>

QCoro::Task<QImage> PodcastCommon::cacheImage(QNetworkAccessManager* mgr, QUrl imageUrl, QString cachedItem) {
    if (!QFile::exists(cachedItem)) {
        QNetworkReply* reply = co_await mgr->get(QNetworkRequest(imageUrl));
        auto error = reply->error();
        if (reply->error() != QNetworkReply::NoError) {
            // Do something!
            throw tException();
        }

        QFile imageFile(cachedItem);
        imageFile.open(QFile::WriteOnly);
        imageFile.write(reply->readAll());
        imageFile.close();
    }

    co_return QImage(cachedItem);
}
