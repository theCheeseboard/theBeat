#ifndef PODCASTCOMMON_H
#define PODCASTCOMMON_H

#include <QCoroTask>

class QNetworkAccessManager;
namespace PodcastCommon {
    QCoro::Task<QImage> cacheImage(QNetworkAccessManager* mgr, QUrl imageUrl, QString cachedItem);
};

#endif // PODCASTCOMMON_H
