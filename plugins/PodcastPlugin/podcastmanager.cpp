#include "podcastmanager.h"

#include "podcast.h"
#include <QCryptographicHash>
#include <QDir>
#include <QStandardPaths>
#include <QUrl>

struct PodcastManagerPrivate {
        QString podcastDir;
        QList<Podcast*> podcasts;
};

PodcastManager* PodcastManager::instance() {
    static auto instance = new PodcastManager();
    return instance;
}

void PodcastManager::init() {
    for (const auto& entry : QDir(d->podcastDir).entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        d->podcasts.append(new Podcast(entry));
    }
    emit podcastsUpdated();
}

Podcast* PodcastManager::subscribe(QUrl rssFeed) {
    auto podcastHash = QCryptographicHash::hash(rssFeed.toString().toUtf8(), QCryptographicHash::Sha256).toHex();

    for (auto podcast : d->podcasts) {
        if (podcast->podcastHash() == podcastHash) return podcast;
    }

    auto podcast = new Podcast(podcastHash);
    podcast->setFeedUrl(rssFeed);
    d->podcasts.append(podcast);

    emit podcastsUpdated();

    return podcast;
}

QList<Podcast*> PodcastManager::podcasts() {
    return d->podcasts;
}

QString PodcastManager::podcastDir() {
    return d->podcastDir;
}

QCoro::Task<> PodcastManager::updatePodcasts() {
    for (auto podcast : d->podcasts) {
        co_await podcast->update();
    }
}

PodcastManager::PodcastManager(QObject* parent) :
    QObject{parent} {
    d = new PodcastManagerPrivate();

    auto appdata = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    d->podcastDir = QDir(appdata).absoluteFilePath("podcasts");

    QDir::root().mkpath(d->podcastDir);
}
