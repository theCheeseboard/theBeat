#include "lastfmapiservice.h"

#include <QCoroNetwork>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QUrlQuery>
#include <tsettings.h>

struct LastFmApiServicePrivate {
        static constexpr const char* baseUrl = "https://api.thebeat.vicr123.com/lastfm";
        QNetworkAccessManager mgr;
        QString apiKey;
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

LastFmApiService::LastFmApiService(QObject* parent) :
    QObject{parent} {
    d = new LastFmApiServicePrivate();
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

LastFmApiException::LastFmApiException(int error, QString reason) {
    _error = error;
    _reason = reason;
}

int LastFmApiException::error() {
    return _error;
}

T_EXCEPTION_IMPL(LastFmApiException)
