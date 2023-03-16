#ifndef PODCASTITEM_H
#define PODCASTITEM_H

#include <QCoroTask>
#include <QObject>

class QXmlStreamReader;
class Podcast;
class QNetworkAccessManager;
struct PodcastItemPrivate;
class PodcastItem : public QObject {
        Q_OBJECT
    public:
        ~PodcastItem();

        Podcast* parentPodcast();

        QString title();
        QString creator();
        QString link();
        QString description();
        QString subtitle();
        QDateTime published();
        QUrl playUrl();
        QString guid();
        QString guidHash();
        QCoro::Task<QImage> image();

        bool isDownloaded();
        QCoro::Task<> download();
        void removeDownload();
        QString downloadedFilePath();

    signals:
        void downloadStateChanged();

    protected:
        friend Podcast;
        explicit PodcastItem(Podcast* parentPodcast, QXmlStreamReader* dataReader, QString podcastDir, QNetworkAccessManager* mgr, QObject* parent = nullptr);

    private:
        PodcastItemPrivate* d;
};

typedef QSharedPointer<PodcastItem> PodcastItemPtr;

#endif // PODCASTITEM_H
