#include "stationlistmodel.h"

#include "radioinfoclient.h"
#include <QException>

struct StationListModelPrivate {
        QList<RadioInfoClient::Station> topVotedStations;
        QList<RadioInfoClient::Station> stations;
        QString query;

        quint16 searchNonce;
};

StationListModel::StationListModel(QObject* parent) :
    QAbstractListModel(parent), d(new StationListModelPrivate()) {
    connect(RadioInfoClient::instance(), &RadioInfoClient::ready, this, [this]() -> QCoro::Task<> {
        try {
            d->topVotedStations = co_await RadioInfoClient::topVoted();

            if (d->query.isEmpty()) {
                this->beginResetModel();
                d->stations = d->topVotedStations;
                this->endResetModel();
            }
        } catch (QException ex) {
        }
    });
}

StationListModel::~StationListModel() {
    delete d;
}

int StationListModel::rowCount(const QModelIndex& parent) const {
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return d->stations.length();
}

QVariant StationListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return QVariant();

    auto station = d->stations.at(index.row());
    switch (role) {
        case Name:
            return station.name;
        case Country:
            return station.country;
        case ImageUrl:
            return station.icon;
        case Url:
            return station.streamUrl;
    }

    return QVariant();
}

void StationListModel::setSearchQuery(QString searchQuery) {
    setSearchQueryCore(searchQuery);
}

QString StationListModel::searchQuery() {
    return d->query;
}

QCoro::Task<> StationListModel::setSearchQueryCore(QString searchQuery) {
    quint16 nonce = ++d->searchNonce;
    d->query = searchQuery;

    if (searchQuery.isEmpty()) {
        this->beginResetModel();
        d->stations = d->topVotedStations;
        this->endResetModel();
    } else {
        this->beginResetModel();
        d->stations.clear();
        this->endResetModel();

        auto stations = co_await RadioInfoClient::search(searchQuery);
        if (d->searchNonce != nonce) co_return;

        this->beginResetModel();
        d->stations = stations;
        this->endResetModel();
    }
}

QHash<int, QByteArray> StationListModel::roleNames() const {
    return {
        {Name,     "name"    },
        {Country,  "country" },
        {ImageUrl, "imageUrl"},
        {Url,      "url"     }
    };
}
