#include "podcastitem.h"

#include "podcastcommon.h"
#include <QCoroNetwork>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QImage>
#include <QMenu>
#include <QMimeDatabase>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QTextDocumentFragment>
#include <QUrl>
#include <QXmlStreamReader>
#include <QtEndian>
#include <ticon.h>
#include <tjobmanager.h>
#include <tqueueguard.h>
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

        quint64 played = 0;

        static tQueueGuard downloadQueueGuard;
        bool isDownloading = false;
};

tQueueGuard PodcastItemPrivate::downloadQueueGuard = tQueueGuard();

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

quint64 PodcastItem::played() {
    return d->played;
}

void PodcastItem::setPlayed(quint64 played) {
    auto wasComplete = this->isCompleted();

    d->played = played;
    auto playedLe = qToLittleEndian(d->played);
    QByteArray bytes(reinterpret_cast<const char*>(&playedLe), sizeof(playedLe));
    QFile playedFile(d->podcastDir.absoluteFilePath("played"));
    playedFile.open(QFile::WriteOnly);
    playedFile.write(bytes);
    playedFile.close();

    if (wasComplete != this->isCompleted()) {
        emit completionStateChanged();
    }
}

bool PodcastItem::isCompleted() {
    return this->duration() - this->played() <= 5;
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

    QPointer<PodcastItem> thisPtr = this;
    auto downloadedFilePath = this->downloadedFilePath();

    auto job = new tStandardJob(transient);
    job->setTitleString(tr("Download Podcast Episode"));
    job->setStatusString(tr("Downloading %1").arg(QLocale().quoteString(d->title)));
    job->setProgress(0);
    job->setTotalProgress(0);
    tJobManager::trackJobDelayed(job);

    auto guardTask = d->downloadQueueGuard.guardQueue();

    auto request = QNetworkRequest(QUrl(d->mediaUrl));
    QNetworkReply* headResponse = co_await d->mgr->head(request);
    job->setTotalProgress(headResponse->header(QNetworkRequest::ContentLengthHeader).toULongLong());

    auto guard = co_await guardTask;

    QFile localFile(QStringLiteral("%1.%2").arg(downloadedFilePath, d->mediaUrlExt));
    localFile.open(QFile::WriteOnly);

    auto reply = d->mgr->get(request);
    connect(reply, &QNetworkReply::downloadProgress, thisPtr, [job](qint64 bytesReceived, qint64 bytesTotal) {
        job->setTotalProgress(bytesTotal);
        job->setProgress(bytesReceived);
    });
    connect(reply, &QNetworkReply::readyRead, thisPtr, [reply, &localFile] {
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

    QFile lockFile(QStringLiteral("%1.lck").arg(downloadedFilePath));
    lockFile.open(QFile::WriteOnly);
    lockFile.close();

    job->setState(tJob::Finished);
    job->setStatusString(tr("Downloaded %1").arg(QLocale().quoteString(d->title)));

    d->isDownloading = false;
    if (thisPtr) {
        emit downloadStateChanged();
    }
}

void PodcastItem::removeDownload() {
    if (!isDownloaded()) return;

    QFile::remove(QStringLiteral("%1.lck").arg(this->downloadedFilePath()));
    QFile::remove(this->downloadedFilePath());
    emit downloadStateChanged();
}

QString PodcastItem::downloadedFilePath() {
    return d->podcastDir.absoluteFilePath(QStringLiteral("audio"));
}

QMenu* PodcastItem::podcastManagementMenu(bool showAllOptions) {
    QMenu* menu = new QMenu();
    menu->addSection(tr("For %1").arg(QLocale().quoteString(this->title())));

    if (showAllOptions) {
        if (isDownloaded()) {
            menu->addAction(tIcon::fromTheme("edit-delete"), tr("Remove Download"), this, [this] {
                this->removeDownload();
            });
        } else if (!isDownloading()) {
            menu->addAction(tIcon::fromTheme("cloud-download"), tr("Download"), this, [this] {
                this->download(false);
            });
        }
    }

    if (this->isCompleted()) {
        menu->addAction(tIcon::fromTheme("view-refresh"), tr("Mark as unplayed"), this, [this] {
            this->setPlayed(0);
        });
    } else {
        menu->addAction(tIcon::fromTheme("dialog-ok"), tr("Mark as complete"), this, [this] {
            this->setPlayed(this->duration());
        });
    }
    return menu;
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

    QFile playedFile(d->podcastDir.absoluteFilePath("played"));
    if (playedFile.exists()) {
        playedFile.open(QFile::ReadOnly);
        auto playedBytes = playedFile.readAll();
        playedFile.close();
        if (playedBytes.length() >= sizeof(d->played)) {
            auto playedLe = *reinterpret_cast<quint64*>(playedBytes.data());
            d->played = qFromLittleEndian(playedLe);
        }
    }
}
