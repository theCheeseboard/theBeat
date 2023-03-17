#include "podcastmanager.h"

#include "podcast.h"
#include <QCryptographicHash>
#include <QDir>
#include <QStandardPaths>
#include <QUrl>
#include <texception.h>
#include <tjobmanager.h>
#include <tlogger.h>
#include <tstandardjob.h>

struct PodcastManagerPrivate {
        QString podcastDir;
        QList<Podcast*> podcasts;
        bool updatingPodcasts = false;
};

PodcastManager* PodcastManager::instance() {
    static auto instance = new PodcastManager();
    return instance;
}

void PodcastManager::init() {
    for (const auto& entry : QDir(d->podcastDir).entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        auto podcast = new Podcast(entry);
        connect(podcast, &Podcast::itemsUpdated, this, &PodcastManager::podcastsUpdated);
        d->podcasts.append(podcast);
    }
    emit podcastsUpdated();
}

Podcast* PodcastManager::subscribe(QUrl rssFeed) {
    auto podcastHash = QCryptographicHash::hash(rssFeed.toString().toUtf8(), QCryptographicHash::Sha256).toHex();

    for (auto podcast : d->podcasts) {
        if (podcast->podcastHash() == podcastHash) return podcast;
    }

    auto podcast = new Podcast(podcastHash);
    connect(podcast, &Podcast::itemsUpdated, this, &PodcastManager::podcastsUpdated);
    podcast->setFeedUrl(rssFeed);
    d->podcasts.append(podcast);

    emit podcastsUpdated();

    return podcast;
}

QList<Podcast*> PodcastManager::podcasts() {
    return d->podcasts;
}

void PodcastManager::unsubscribe(Podcast* podcast) {
    if (podcast->unsubscribe()) {
        d->podcasts.removeAll(podcast);
        emit podcastsUpdated();
    }
}

QString PodcastManager::podcastDir() {
    return d->podcastDir;
}

QCoro::Task<> PodcastManager::updatePodcasts(bool transient) {
    if (d->updatingPodcasts) co_return;

    d->updatingPodcasts = true;
    auto job = new tStandardJob(transient);
    job->setTitleString(tr("Updating Podcasts"));
    tJobManager::trackJob(job);
    for (auto podcast : d->podcasts) {
        job->setStatusString(tr("Updating %1").arg(QLocale().quoteString(podcast->name())));
        try {
            co_await podcast->update();
        } catch (tException& ex) {
            // ignore
            tWarn("PodcastManager") << "Failed to update podcast " << podcast->name();
        }
    }

    job->setState(tJob::Finished);
    job->setStatusString(tr("Updated Podcasts"));
    d->updatingPodcasts = false;
}

PodcastManager::PodcastManager(QObject* parent) :
    QObject{parent} {
    d = new PodcastManagerPrivate();

    auto appdata = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    d->podcastDir = QDir(appdata).absoluteFilePath("podcasts");

    QDir::root().mkpath(d->podcastDir);
}
