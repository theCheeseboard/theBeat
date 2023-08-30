#include "lastfmapiservice.h"

#include <QCoroNetwork>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QUrlQuery>
#include <ranges/trange.h>
#include <tlogger.h>
#include <tsettings.h>

struct LastFmApiServicePrivate {
    static constexpr const char* baseUrl = "https://api.thebeat.vicr123.com/lastfm";
    QNetworkAccessManager mgr;
    QString apiKey;

    QList<LastFmApiService::Scrobble> pendingScrobbles;
    bool sendingScrobbles = false;
};

QString LastFmApiService::apiKey() {
    return LastFmApiService::instance()->d->apiKey;
}

QString LastFmApiService::loggedInUser() {
    tSettings settings;
    return settings.value("lastfm/username").toString();
}

QString LastFmApiService::loggedInSessionKey() {
    tSettings settings;
    return settings.value("lastfm/sessionkey").toString();
}

void LastFmApiService::logout() {
    // Clear out any pending scrobbles since we're logging out
    instance()->d->pendingScrobbles.clear();

    tSettings settings;
    settings.setValue("lastfm/username", "");
    settings.setValue("lastfm/sessionkey", "");
}

QCoro::Task<QString> LastFmApiService::getUnauthenticatedToken() {
    auto response = co_await LastFmApiService::instance()->get("auth.getToken", {});
    co_return response.value("token").toString();
}

QCoro::Task<> LastFmApiService::attemptLoginWithToken(QString token) {
    auto response = co_await LastFmApiService::instance()->get("auth.getSession", {
        {"token", token}
    });

    auto session = response.value("session").toObject();

    tSettings settings;
    settings.setValue("lastfm/username", session.value("name").toString());
    settings.setValue("lastfm/sessionkey", session.value("key").toString());
}

QCoro::Task<> LastFmApiService::scrobble() {
    instance();

    auto sk = LastFmApiService::loggedInSessionKey();
    if (sk.isEmpty()) co_return;

    instance()->d->sendingScrobbles = true;

    try {
        auto toScrobble = tRange(instance()->d->pendingScrobbles).take(50).toList();

        if (toScrobble.isEmpty()) {
            instance()->d->sendingScrobbles = false;
            co_return;
        }

        QJsonObject object;
        for (auto i = 0; i < toScrobble.length(); i++) {
            toScrobble.at(i).write(&object, i);
        }
        object.insert("sk", sk);

        tDebug("LastFmApiService") << "Sending " << toScrobble.length() << " scrobble(s)";
        auto response = co_await LastFmApiService::instance()->post("track.scrobble", object);

        // If we get here, scrobbling was successful so remove the sent scrobbles
        instance()->d->pendingScrobbles.remove(0, toScrobble.length());
        tDebug("LastFmApiService") << "Sent " << toScrobble.length() << " scrobble(s)";
    } catch (LastFmApiException& ex) {
        tWarn("LastFmApiService") << "Sending scrobbles failed:";
        tWarn("LastFmApiService") << "Error " << ex.error();
        tWarn("LastFmApiService") << "Reason: " << ex.reason();
        tWarn("LastFmApiService") << instance()->d->pendingScrobbles.length() << " scrobble(s) pending.";
    }

    instance()->d->sendingScrobbles = false;
}

void LastFmApiService::pushScrobble(Scrobble scrobble) {
    instance()->d->pendingScrobbles.append(scrobble);
    instance()->savePendingScrobbles();

    LastFmApiService::scrobble();
}

LastFmApiService::LastFmApiService(QObject* parent) :
    QObject{parent} {
    d = new LastFmApiServicePrivate();
    this->loadPendingScrobbles();
}

LastFmApiService* LastFmApiService::instance() {
    static auto instance = new LastFmApiService();
    return instance;
}

QCoro::Task<QJsonObject> LastFmApiService::get(QString method, QMap<QString, QString> arguments) {
    QUrl url(LastFmApiServicePrivate::baseUrl);
    QUrlQuery query({
        {"method", method}
    });

    for (const auto& arg : arguments.keys()) {
        query.addQueryItem(arg, arguments.value(arg));
    }

    url.setQuery(query);

    auto reply = co_await d->mgr.get(QNetworkRequest(url));
    d->apiKey = reply->rawHeader("X-theBeat-Api-Key");

    // TODO: Error handling

    auto json = QJsonDocument::fromJson(reply->readAll()).object();
    if (json.contains("error")) {
        throw LastFmApiException(json.value("error").toInt(), json.value("reason").toString());
    }
    co_return json;
}

QCoro::Task<QJsonObject> LastFmApiService::post(QString method, QJsonObject arguments) {
    QUrl url(LastFmApiServicePrivate::baseUrl);
    arguments.insert("method", method);

    auto reply = co_await d->mgr.post(QNetworkRequest(url), QJsonDocument(arguments).toJson(QJsonDocument::Compact));
    d->apiKey = reply->rawHeader("X-theBeat-Api-Key");

    // TODO: Error handling

    auto json = QJsonDocument::fromJson(reply->readAll()).object();
    if (json.contains("error")) {
        throw LastFmApiException(json.value("error").toInt(), json.value("reason").toString());
    }
    co_return json;
}

void LastFmApiService::loadPendingScrobbles() {
    tSettings settings;
    d->pendingScrobbles = tRange(settings.delimitedList("lastfm/pending")).filter([](const QString & pendingScrobbleData) {
        return !pendingScrobbleData.isEmpty();
    })
    .map<Scrobble>([](const QString & scrobbleData) {
        return Scrobble(QJsonDocument::fromJson(QByteArray::fromBase64(scrobbleData.toUtf8())).object());
    })
    .toList();
}

void LastFmApiService::savePendingScrobbles() {
    tSettings settings;
    settings.setDelimitedList("lastfm/pending", tRange(d->pendingScrobbles).map<QString>([](const Scrobble & scrobble) {
        QJsonObject obj;
        scrobble.write(&obj, 0);
        return QJsonDocument(obj).toJson(QJsonDocument::Compact).toBase64();
    })
    .toList());
}

LastFmApiException::LastFmApiException(int error, QString reason) {
    _error = error;
    _reason = reason;
}

int LastFmApiException::error() {
    return _error;
}

LastFmApiService::Scrobble::Scrobble(QJsonObject object) {
    artist = object.value("artist[0]").toString();
    track = object.value("track[0]").toString();
    timestamp = object.value("timestamp[0]").toString();
}

void LastFmApiService::Scrobble::write(QJsonObject* object, int index) const {
    object->insert(QStringLiteral("artist[%1]").arg(index), artist);
    object->insert(QStringLiteral("track[%1]").arg(index), track);
    object->insert(QStringLiteral("timestamp[%1]").arg(index), timestamp);
}

T_EXCEPTION_IMPL(LastFmApiException)
