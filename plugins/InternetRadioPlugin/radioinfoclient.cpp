/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2021 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "radioinfoclient.h"

#include <QNetworkAccessManager>
#include <QRandomGenerator>
#include <QHostInfo>
#include <QNetworkReply>
#include <QDnsLookup>
#include <tapplication.h>

struct RadioInfoClientPrivate {
    QHostAddress selectedAddress;
    QNetworkAccessManager mgr;

    QMap<QString, QPixmap> iconCache;
};

RadioInfoClient* RadioInfoClient::instance() {
    static RadioInfoClient* instance = new RadioInfoClient();
    return instance;
}

tPromise<void>* RadioInfoClient::selectServer() {
    return TPROMISE_CREATE_SAME_THREAD(void, {
        QDnsLookup* lookup = new QDnsLookup(QDnsLookup::A, "all.api.radio-browser.info");
        connect(lookup, &QDnsLookup::finished, [ = ] {
            if (lookup->error() != QDnsLookup::NoError) {
                rej(lookup->errorString());
                lookup->deleteLater();
                return;
            }

            instance()->d->selectedAddress = lookup->hostAddressRecords().at(QRandomGenerator::system()->bounded(lookup->hostAddressRecords().count())).value();
            emit instance()->ready();
            res();
        });
        lookup->lookup();
    });
}

tPromise<QList<RadioInfoClient::Station>>* RadioInfoClient::topVoted() {
//    return tPromise<QList<RadioInfoClient::Station>>::runOnSameThread([ = ](tPromiseFunctions<QList<Station>>::SuccessFunction res, tPromiseFunctions<QList<Station>>::FailureFunction rej) {
    return TPROMISE_CREATE_SAME_THREAD(QList<Station>, {
        QUrl url;
        url.setScheme("http");
        url.setHost(instance()->d->selectedAddress.toString());
        url.setPath("/json/stations/topvote/25");

        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("%1/%2").arg(tApplication::applicationName(), tApplication::applicationVersion()));
        QNetworkReply* reply = instance()->d->mgr.get(req);
        connect(reply, &QNetworkReply::finished, [ = ] {
            if (reply->error() != QNetworkReply::NoError) {
                rej(reply->errorString());
                return;
            }

            QByteArray data = reply->readAll();
            QJsonArray array = QJsonDocument::fromJson(data).array();

            QList<Station> stations;
            for (const QJsonValue& station : array) {
                stations.append(Station(station.toObject(), &instance()->d->iconCache));
            }
            res(stations);
        });
    });
}

tPromise<QList<RadioInfoClient::Station>>* RadioInfoClient::search(QString query) {
    return TPROMISE_CREATE_SAME_THREAD(QList<Station>, {
        QUrl url;
        url.setScheme("http");
        url.setHost(instance()->d->selectedAddress.toString());
        url.setPath("/json/stations/search");

        QUrlQuery queryString;
        queryString.addQueryItem("name", query);
        queryString.addQueryItem("limit", "50");
        url.setQuery(queryString);

        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("%1/%2").arg(tApplication::applicationName(), tApplication::applicationVersion()));
        QNetworkReply* reply = instance()->d->mgr.get(req);
        connect(reply, &QNetworkReply::finished, [ = ] {
            if (reply->error() != QNetworkReply::NoError) {
                rej(reply->errorString());
                return;
            }

            QByteArray data = reply->readAll();
            QJsonArray array = QJsonDocument::fromJson(data).array();

            QList<Station> stations;
            for (const QJsonValue& station : array) {
                stations.append(Station(station.toObject(), &instance()->d->iconCache));
            }
            res(stations);
        });
    });
}

tPromise<QPixmap>* RadioInfoClient::getIcon(RadioInfoClient::Station station) {
    return TPROMISE_CREATE_SAME_THREAD(QPixmap, {
        QString stationUuid = station.stationUuid;
        if (instance()->d->iconCache.contains(stationUuid)) {
            QPixmap px = instance()->d->iconCache.value(stationUuid);
            if (px.isNull()) {
                rej("null");
            } else {
                res(px);
            }
            return;
        }

        QNetworkAccessManager* mgr = new QNetworkAccessManager();
        QNetworkRequest req((QUrl(station.icon)));
        req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("%1/%2").arg(tApplication::applicationName(), tApplication::applicationVersion()));
        QNetworkReply* reply = mgr->get(req);
        connect(reply, &QNetworkReply::finished, [ = ] {
            if (reply->error() != QNetworkReply::NoError) {
                rej(reply->errorString());
                mgr->deleteLater();
                return;
            }

            QPixmap px;
            px.loadFromData(reply->readAll());
            instance()->d->iconCache.insert(stationUuid, px);
            if (px.isNull()) {
                rej("null");
            } else {
                res(px);
            }
            mgr->deleteLater();
        });
    });
}

void RadioInfoClient::countClick(RadioInfoClient::Station station) {
    QUrl url;
    url.setScheme("http");
    url.setHost(instance()->d->selectedAddress.toString());
    url.setPath(QStringLiteral("/json/url/%1").arg(station.stationUuid));

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("%1/%2").arg(tApplication::applicationName(), tApplication::applicationVersion()));
    instance()->d->mgr.get(req);
}

RadioInfoClient::RadioInfoClient(QObject* parent) : QObject(parent) {
    d = new RadioInfoClientPrivate();

    selectServer();
}

RadioInfoClient::Station::Station() {

}

RadioInfoClient::Station::Station(QJsonObject object, QMap<QString, QPixmap>* iconCache) {
    this->iconCache = iconCache;

    stationUuid = object.value("stationuuid").toString();
    name = object.value("name").toString();
    streamUrl = object.value("url").toString();
    icon = object.value("favicon").toString();
    country = object.value("country").toString();

    json = object;
}
