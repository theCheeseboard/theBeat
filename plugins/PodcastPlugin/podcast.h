#ifndef PODCAST_H
#define PODCAST_H

#include <QCoroTask>
#include <QObject>

class PodcastManager;
class PodcastItem;
class QMenu;
class QNetworkAccessManager;
struct PodcastPrivate;
typedef QSharedPointer<PodcastItem> PodcastItemPtr;
class Podcast : public QObject {
        Q_OBJECT
    public:
        ~Podcast();

        QString podcastHash();

        QString name();
        QCoro::Task<QImage> image();
        QList<PodcastItemPtr> items();

        QCoro::Task<> update();

        QMenu* podcastManagementMenu();

        bool unsubscribe();

    signals:
        void itemsUpdated();
        void unsubscribed();

    protected:
        friend PodcastManager;
        explicit Podcast(QString podcastHash, QNetworkAccessManager* mgr);

        void setFeedUrl(QUrl url);

    private:
        PodcastPrivate* d;

        void readXml();
};

#endif // PODCAST_H
