#include "podcast.h"

#include "podcastcommon.h"
#include "podcastitem.h"
#include "podcastmanager.h"
#include <QCoroNetwork>
#include <QDir>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QXmlStreamReader>
#include <texception.h>
#include <tlogger.h>

struct PodcastPrivate {
        QNetworkAccessManager mgr;
        QString podcastHash;
        QDir podcastDir;
        QUrl feedUrl;

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

Podcast::Podcast(QString podcastHash) :
    QObject{nullptr} {
    d = new PodcastPrivate();
    d->podcastHash = podcastHash;

    QDir rootPodcastDir = PodcastManager::instance()->podcastDir();
    rootPodcastDir.mkdir(podcastHash);
    d->podcastDir = rootPodcastDir.absoluteFilePath(podcastHash);
    d->podcastDir.mkdir("audio");
    d->podcastDir.mkdir("images");

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
                d->items.append(PodcastItemPtr(new PodcastItem(this, &reader, d->podcastDir.absolutePath(), &d->mgr)));
            } else {
                reader.skipCurrentElement();
            }
        }
    }

    if (reader.error() != QXmlStreamReader::NoError) {
        tWarn("Podcast") << "Podcast parse error: " << reader.errorString();
        return;
    }

    tDebug("Podcast") << "Podcast Name: " << d->podcastName;
    tDebug("Podcast") << "Podcast Link: " << d->podcastLink;
    tDebug("Podcast") << "Podcast Author: " << d->podcastAuthor;
    tDebug("Podcast") << "Podcast Subtitle: " << d->podcastSubtitle;
    tDebug("Podcast") << "Podcast Copyright: " << d->podcastCopyright;
    tDebug("Podcast") << "Podcast Image: " << d->podcastImageUrl;
    tDebug("Podcast") << "Podcast Description: " << d->podcastDescription;

    for (auto item : d->items) {
        if (item->link().isEmpty()) continue;

        tDebug("Podcast") << "Item Title: " << item->title();
        tDebug("Podcast") << "Item Creator: " << item->creator();
        tDebug("Podcast") << "Item Link: " << item->link();
        tDebug("Podcast") << "Item Description: " << item->description();
        tDebug("Podcast") << "Item Subtitle: " << item->subtitle();
        tDebug("Podcast") << "Item Published: " << item->published().toString();
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
    co_return co_await PodcastCommon::cacheImage(&d->mgr, d->podcastImageUrl, d->podcastDir.absoluteFilePath("cover"));
}

QList<PodcastItemPtr> Podcast::items() {
    return d->items;
}

QCoro::Task<> Podcast::update() {
    QNetworkReply* reply = co_await d->mgr.get(QNetworkRequest(d->feedUrl));
    auto error = reply->error();
    if (reply->error() != QNetworkReply::NoError) {
        // Do something!
        co_return;
    }

    QFile xmlFile(d->podcastDir.absoluteFilePath("feed.xml"));
    xmlFile.open(QFile::WriteOnly);
    xmlFile.write(reply->readAll());
    xmlFile.close();

    readXml();
}
