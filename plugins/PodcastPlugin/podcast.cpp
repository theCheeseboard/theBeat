#include "podcast.h"

#include "podcastcommon.h"
#include "podcastitem.h"
#include "podcastmanager.h"
#include <QCoroNetwork>
#include <QDir>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QXmlStreamReader>
#include <texception.h>
#include <tlogger.h>
#include <tsettings.h>

struct PodcastPrivate {
        QNetworkAccessManager* mgr;
        QString podcastHash;
        QDir podcastDir;
        QUrl feedUrl;

        tSettings settings;

        QString podcastName;
        QString podcastLink;
        QString podcastAuthor;
        QString podcastSubtitle;
        bool podcastExplicit;
        QString podcastCopyright;
        QString podcastImageUrl;
        QString podcastDescription;

        QList<PodcastItemPtr> items;
};

Podcast::Podcast(QString podcastHash, QNetworkAccessManager* mgr) :
    QObject{nullptr} {
    d = new PodcastPrivate();
    d->podcastHash = podcastHash;
    d->mgr = mgr;

    QDir rootPodcastDir = PodcastManager::instance()->podcastDir();
    rootPodcastDir.mkdir(podcastHash);
    d->podcastDir = rootPodcastDir.absoluteFilePath(podcastHash);

    if (QFile::exists(d->podcastDir.absoluteFilePath("feedUrl"))) {
        QFile feedFile(d->podcastDir.absoluteFilePath("feedUrl"));
        feedFile.open(QFile::ReadOnly);
        d->feedUrl = QUrl(feedFile.readAll().trimmed());
        feedFile.close();
    }

    if (QFile::exists(d->podcastDir.absoluteFilePath("feed.xml"))) {
        readXml();
    }
}

void Podcast::setFeedUrl(QUrl url) {
    QFile feedFile(d->podcastDir.absoluteFilePath("feedUrl"));
    feedFile.open(QFile::WriteOnly);
    feedFile.write(url.toString().toUtf8());
    feedFile.close();
    d->feedUrl = url;
}

void Podcast::readXml() {
    d->items.clear();

    QFile xmlFile(d->podcastDir.absoluteFilePath("feed.xml"));
    xmlFile.open(QFile::ReadOnly);

    const auto itunesNamespace = QStringLiteral("http://www.itunes.com/dtds/podcast-1.0.dtd");

    QXmlStreamReader reader(&xmlFile);
    reader.readNextStartElement();
    if (reader.name() != QStringLiteral("rss")) return;
    reader.readNextStartElement();
    if (reader.name() != QStringLiteral("channel")) return;

    while (reader.readNextStartElement()) { // channel
        auto ns = reader.namespaceUri();
        auto name = reader.name();
        if (reader.namespaceUri() == itunesNamespace) {
            if (reader.name() == QStringLiteral("author")) {
                d->podcastAuthor = reader.readElementText();
            } else if (reader.name() == QStringLiteral("subtitle")) {
                d->podcastSubtitle = reader.readElementText();
            } else if (reader.name() == QStringLiteral("explicit")) {
                d->podcastExplicit = reader.readElementText() == "yes";
            } else if (reader.name() == QStringLiteral("image")) {
                d->podcastImageUrl = reader.attributes().value("href").toString();
                reader.skipCurrentElement();
            } else {
                reader.skipCurrentElement();
            }
        } else {
            if (reader.name() == QStringLiteral("title")) {
                d->podcastName = reader.readElementText();
            } else if (reader.name() == QStringLiteral("link")) {
                d->podcastLink = reader.readElementText();
            } else if (reader.name() == QStringLiteral("copyright")) {
                d->podcastCopyright = reader.readElementText();
            } else if (reader.name() == QStringLiteral("description")) {
                d->podcastDescription = reader.readElementText();
            } else if (reader.name() == QStringLiteral("item")) {
                // Podcast item
                d->items.append(PodcastItemPtr(new PodcastItem(this, &reader, d->podcastDir.absolutePath(), d->mgr)));
            } else {
                reader.skipCurrentElement();
            }
        }
    }

    if (reader.error() != QXmlStreamReader::NoError) {
        tWarn("Podcast") << "Podcast parse error: " << reader.errorString();
        return;
    }

    emit itemsUpdated();
}

Podcast::~Podcast() {
    delete d;
}

QString Podcast::podcastHash() {
    return d->podcastHash;
}

QString Podcast::name() {
    return d->podcastName;
}

QCoro::Task<QImage> Podcast::image() {
    co_return co_await PodcastCommon::cacheImage(d->mgr, d->podcastImageUrl, d->podcastDir.absoluteFilePath("cover"));
}

QList<PodcastItemPtr> Podcast::items() {
    return d->items;
}

QCoro::Task<> Podcast::update() {
    QNetworkReply* reply = co_await d->mgr->get(QNetworkRequest(d->feedUrl));
    auto error = reply->error();
    if (reply->error() != QNetworkReply::NoError) {
        // Do something!
        co_return;
    }

    auto feedPath = d->podcastDir.absoluteFilePath("feed.xml");
    auto initialDownload = !QFile::exists(feedPath);

    QStringList knownGuids;
    for (auto item : d->items) {
        knownGuids.append(item->guid());
    }

    QFile xmlFile(feedPath);
    xmlFile.open(QFile::WriteOnly);
    xmlFile.write(reply->readAll());
    xmlFile.close();

    readXml();

    if (initialDownload) {
        // Mark all the episodes as played
        for (auto item : d->items) {
            item->setPlayed(item->duration());
        }
    } else {
        if (d->settings.value("podcasts/autoDownload").toBool()) {
            // Start downloading new podcast episodes from oldest to newest
            for (auto i = d->items.rbegin(); i != d->items.rend(); i++) {
                auto item = *i;
                if (!item->isDownloading() && !item->isDownloaded() && !knownGuids.contains(item->guid())) {
                    item->download(true);
                }
            }
        }
    }
}

QMenu* Podcast::podcastManagementMenu() {
    QMenu* menu = new QMenu();
    menu->addSection(tr("For %1").arg(QLocale().quoteString(this->name())));

    QMenu* removeMenu = new QMenu();
    removeMenu->setIcon(QIcon::fromTheme("edit-delete"));
    removeMenu->setTitle(tr("Unsubscribe"));
    removeMenu->addSection(tr("Are you sure?"));
    removeMenu->addAction(QIcon::fromTheme("edit-delete"), tr("Unsubscribe"), this, [this] {
        PodcastManager::instance()->unsubscribe(this);
    });
    menu->addMenu(removeMenu);
    return menu;
}

bool Podcast::unsubscribe() {
    if (d->podcastDir.removeRecursively()) {
        emit unsubscribed();
        return true;
    } else {
        return false;
    }
}
