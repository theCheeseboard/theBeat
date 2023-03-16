#include "podcastitem.h"

#include <QCoroNetwork>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QMimeDatabase>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QXmlStreamReader>

struct PodcastItemPrivate {
        QNetworkAccessManager* mgr;
        QDir podcastDir;

        QString title;
        QString creator;
        QString link;
        QString description;
        QString author;
        QString subtitle;
        bool isExplicit;
        QString imageUrl;
        QString mediaUrl;
        QString mediaUrlExt;
        QDateTime published;
        QString guid;
};

PodcastItem::~PodcastItem() {
    delete d;
}

QString PodcastItem::title() {
    return d->title;
}

QString PodcastItem::creator() {
    return d->creator;
}

QString PodcastItem::link() {
    return d->link;
}

QString PodcastItem::description() {
    return d->description;
}

QString PodcastItem::subtitle() {
    return d->subtitle;
}

QDateTime PodcastItem::published() {
    return d->published;
}

QUrl PodcastItem::playUrl() {
    // TODO: Downloads
    if (this->isDownloaded()) {
        return QUrl::fromLocalFile(QStringLiteral("%1.%2").arg(this->downloadedFilePath(), d->mediaUrlExt));
    }
    return d->mediaUrl;
}

QString PodcastItem::guid() {
    if (!d->guid.isEmpty()) return d->guid;
    return d->mediaUrl;
}

QString PodcastItem::guidHash() {
    return QCryptographicHash::hash(this->guid().toUtf8(), QCryptographicHash::Sha256).toHex();
}

bool PodcastItem::isDownloaded() {
    return QFile::exists(QStringLiteral("%1.lck").arg(this->downloadedFilePath()));
}

QCoro::Task<> PodcastItem::download() {
    if (isDownloaded()) co_return;
    QFile localFile(QStringLiteral("%1.%2").arg(this->downloadedFilePath(), d->mediaUrlExt));
    localFile.open(QFile::WriteOnly);

    auto reply = d->mgr->get(QNetworkRequest(QUrl(d->mediaUrl)));
    connect(reply, &QNetworkReply::readyRead, this, [this, reply, &localFile] {
        localFile.write(reply->readAll());
    });
    co_await reply;

    auto error = reply->error();
    if (reply->error() != QNetworkReply::NoError) {
        // Do something!
        co_return;
    }

    localFile.write(reply->readAll());
    localFile.close();

    QFile lockFile(QStringLiteral("%1.lck").arg(this->downloadedFilePath()));
    lockFile.open(QFile::WriteOnly);
    lockFile.close();

    emit downloadStateChanged();
}

void PodcastItem::removeDownload() {
    if (!isDownloaded()) return;

    QFile::remove(QStringLiteral("%1.lck").arg(this->downloadedFilePath()));
    QFile::remove(this->downloadedFilePath());
    emit downloadStateChanged();
}

QString PodcastItem::downloadedFilePath() {
    return d->podcastDir.absoluteFilePath(QStringLiteral("audio/%1").arg(this->guidHash()));
}

PodcastItem::PodcastItem(QXmlStreamReader* dataReader, QString podcastDir, QNetworkAccessManager* mgr, QObject* parent) :
    QObject{parent} {
    d = new PodcastItemPrivate();
    d->podcastDir = podcastDir;
    d->mgr = mgr;

    const auto itunesNamespace = QStringLiteral("http://www.itunes.com/dtds/podcast-1.0.dtd");
    const auto contentNamespace = QStringLiteral("http://purl.org/rss/1.0/modules/content/");
    const auto dcNamespace = QStringLiteral("http://purl.org/dc/elements/1.1/");

    while (dataReader->readNextStartElement()) {
        if (dataReader->namespaceUri() == itunesNamespace) {
            if (dataReader->name() == QStringLiteral("author")) {
                d->author = dataReader->readElementText();
            } else if (dataReader->name() == QStringLiteral("subtitle")) {
                d->subtitle = dataReader->readElementText();
            } else if (dataReader->name() == QStringLiteral("explicit")) {
                d->isExplicit = dataReader->readElementText() == "yes";
            } else if (dataReader->name() == QStringLiteral("image")) {
                d->imageUrl = dataReader->attributes().value("href").toString();
                dataReader->skipCurrentElement();
            } else {
                dataReader->skipCurrentElement();
            }
        } else if (dataReader->namespaceUri() == contentNamespace) {
            if (dataReader->name() == QStringLiteral("encoded")) {
                d->description = dataReader->readElementText();
            } else {
                dataReader->skipCurrentElement();
            }
        } else if (dataReader->namespaceUri() == dcNamespace) {
            if (dataReader->name() == QStringLiteral("creator")) {
                d->creator = dataReader->readElementText();
            } else {
                dataReader->skipCurrentElement();
            }
        } else {
            if (dataReader->name() == QStringLiteral("title")) {
                d->title = dataReader->readElementText();
            } else if (dataReader->name() == QStringLiteral("link")) {
                d->link = dataReader->readElementText();
            } else if (dataReader->name() == QStringLiteral("description")) {
                d->description = dataReader->readElementText();
            } else if (dataReader->name() == QStringLiteral("guid")) {
                d->guid = dataReader->readElementText();
            } else if (dataReader->name() == QStringLiteral("pubDate")) {
                d->published = QDateTime::fromString(dataReader->readElementText(), Qt::RFC2822Date);
            } else if (dataReader->name() == QStringLiteral("enclosure")) {
                d->mediaUrl = dataReader->attributes().value("url").toString();

                QMimeDatabase mimeDb;
                auto mimeType = mimeDb.mimeTypeForName(dataReader->attributes().value("type").toString());
                d->mediaUrlExt = mimeType.preferredSuffix();
                dataReader->skipCurrentElement();
            } else {
                dataReader->skipCurrentElement();
            }
        }
    }
}
