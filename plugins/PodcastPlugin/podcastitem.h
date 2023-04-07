#ifndef PODCASTITEM_H
#define PODCASTITEM_H

#include <QCoroTask>
#include <QMenu>
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
        QString plainDescription();
        QString subtitle();
        QDateTime published();
        QUrl playUrl();
        QString guid();
        QString guidHash();
        quint64 duration();
        QCoro::Task<QImage> image();

        quint64 played();
        void setPlayed(quint64 played);
        bool isCompleted();

        bool isDownloaded();
        bool isDownloading();
        QCoro::Task<> download(bool transient);
        void removeDownload();
        QString downloadedFilePath();

        QMenu* podcastManagementMenu(bool showAllOptions);

    signals:
        void downloadStateChanged();
        void completionStateChanged();

    protected:
        friend Podcast;
        explicit PodcastItem(Podcast* parentPodcast, QXmlStreamReader* dataReader, QString podcastDir, QNetworkAccessManager* mgr, QObject* parent = nullptr);

    private:
        PodcastItemPrivate* d;
};

typedef QSharedPointer<PodcastItem> PodcastItemPtr;

#endif // PODCASTITEM_H
