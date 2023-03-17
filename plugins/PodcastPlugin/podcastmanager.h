#ifndef PODCASTMANAGER_H
#define PODCASTMANAGER_H

#include <QCoroTask>
#include <QObject>

class Podcast;
struct PodcastManagerPrivate;
class PodcastManager : public QObject {
        Q_OBJECT
    public:
        static PodcastManager* instance();

        void init();

        Podcast* subscribe(QUrl rssFeed);
        QList<Podcast*> podcasts();

        QString podcastDir();

        QCoro::Task<> updatePodcasts(bool transient);

    signals:
        void podcastsUpdated();

    private:
        explicit PodcastManager(QObject* parent = nullptr);

        PodcastManagerPrivate* d;
};

#endif // PODCASTMANAGER_H
