#include "podcastitem.h"

#include "podcastcommon.h"
#include <QCoroNetwork>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QImage>
#include <QMimeDatabase>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QTextDocumentFragment>
#include <QUrl>
#include <QXmlStreamReader>
#include <tjobmanager.h>
#include <tstandardjob.h>

struct PodcastItemPrivate {
        QNetworkAccessManager* mgr;
        Podcast* parentPodcast;
        QDir podcastDir;

        QString title;
        QString creator;
        QString link;
        QString description;
        QString plainDescription;
        QString author;
        QString subtitle;
        bool isExplicit;
        quint64 duration;
        QString imageUrl;
        QString mediaUrl;
        QString mediaUrlExt;
        QDateTime published;
        QString guid;

        bool isDownloading = false;
};

PodcastItem::~PodcastItem() {
    delete d;
}

Podcast* PodcastItem::parentPodcast() {
    return d->parentPodcast;
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

QString PodcastItem::plainDescription() {
    return d->plainDescription;
}

QString PodcastItem::subtitle() {
    return d->subtitle;
}

QDateTime PodcastItem::published() {
    return d->published;
}

QUrl PodcastItem::playUrl() {
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

quint64 PodcastItem::duration() {
    return d->duration;
}

QCoro::Task<QImage> PodcastItem::image() {
    co_return co_await PodcastCommon::cacheImage(d->mgr, QUrl(d->imageUrl), d->podcastDir.absoluteFilePath(QStringLiteral("cover")));
}

bool PodcastItem::isDownloaded() {
    return QFile::exists(QStringLiteral("%1.lck").arg(this->downloadedFilePath()));
}

bool PodcastItem::isDownloading() {
    return d->isDownloading;
}

QCoro::Task<> PodcastItem::download(bool transient) {
    if (isDownloaded() || isDownloading()) co_return;
    d->isDownloading = true;
    emit downloadStateChanged();

    auto job = new tStandardJob(transient);
    job->setTitleString(tr("Download Podcast Episode"));
    job->setStatusString(tr("Downloading %1").arg(QLocale().quoteString(d->title)));
    job->setProgress(0);
    job->setTotalProgress(0);
    tJobManager::trackJobDelayed(job);

    QFile localFile(QStringLiteral("%1.%2").arg(this->downloadedFilePath(), d->mediaUrlExt));
    localFile.open(QFile::WriteOnly);

    auto reply = d->mgr->get(QNetworkRequest(QUrl(d->mediaUrl)));
    connect(reply, &QNetworkReply::downloadProgress, this, [job](qint64 bytesReceived, qint64 bytesTotal) {
        job->setTotalProgress(bytesTotal);
        job->setProgress(bytesReceived);
    });
    connect(reply, &QNetworkReply::readyRead, this, [this, reply, &localFile] {
        localFile.write(reply->readAll());
    });
    co_await reply;

    auto error = reply->error();
    if (reply->error() != QNetworkReply::NoError) {
        // Do something!
        job->setState(tJob::Failed);
        job->setStatusString(tr("Failed to download %1").arg(QLocale().quoteString(d->title)));

        d->isDownloading = false;
        emit downloadStateChanged();
        co_return;
    }

    localFile.write(reply->readAll());
    localFile.close();

    QFile lockFile(QStringLiteral("%1.lck").arg(this->downloadedFilePath()));
    lockFile.open(QFile::WriteOnly);
    lockFile.close();

    job->setState(tJob::Finished);
    job->setStatusString(tr("Downloaded %1").arg(QLocale().quoteString(d->title)));

    d->isDownloading = false;
    emit downloadStateChanged();
}

void PodcastItem::removeDownload() {
    if (!isDownloaded()) return;

    QFile::remove(QStringLiteral("%1.lck").arg(this->downloadedFilePath()));
    QFile::remove(this->downloadedFilePath());
    emit downloadStateChanged();
}

QString PodcastItem::downloadedFilePath() {
    return d->podcastDir.absoluteFilePath(QStringLiteral("audio").arg(this->guidHash()));
}

PodcastItem::PodcastItem(Podcast* parentPodcast, QXmlStreamReader* dataReader, QString podcastDir, QNetworkAccessManager* mgr, QObject* parent) :
    QObject{parent} {
    d = new PodcastItemPrivate();
    d->parentPodcast = parentPodcast;
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
            } else if (dataReader->name() == QStringLiteral("duration")) {
                auto durationString = dataReader->readElementText();
                auto hms = QRegularExpression(R"(^(\d{1,2}):(\d{1,2}):(\d{1,2})$)").match(durationString);
                auto ms = QRegularExpression(R"(^(\d{1,2}):(\d{1,2})$)").match(durationString);

                quint64 h = 0, m = 0, s = 0;
                if (hms.hasMatch()) {
                    h = hms.capturedView(1).toULongLong();
                    m = hms.capturedView(2).toULongLong();
                    s = hms.capturedView(3).toULongLong();
                } else if (ms.hasMatch()) {
                    m = hms.capturedView(1).toULongLong();
                    s = hms.capturedView(2).toULongLong();
                } else {
                    s = durationString.toULongLong();
                }

                m = m + h * 60;
                s = s + m * 60;
                d->duration = s;
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
                d->plainDescription = QTextDocumentFragment::fromHtml(d->description).toPlainText();
            } else if (dataReader->name() == QStringLiteral("guid")) {
                d->guid = dataReader->readElementText();
            } else if (dataReader->name() == QStringLiteral("pubDate")) {
                auto date = dataReader->readElementText();
                date.replace("GMT", "+0000");
                d->published = QDateTime::fromString(date, Qt::RFC2822Date);
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

    d->podcastDir = QDir(podcastDir).absoluteFilePath(this->guidHash());
    QDir::root().mkpath(d->podcastDir.absolutePath());
}
