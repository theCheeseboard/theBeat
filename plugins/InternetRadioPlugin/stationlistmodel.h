#ifndef STATIONLISTMODEL_H
#define STATIONLISTMODEL_H

#include <QAbstractListModel>
#include <QCoroTask>
#include <QQmlComponent>

struct StationListModelPrivate;
class StationListModel : public QAbstractListModel {
        Q_OBJECT
        Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged FINAL)
        QML_ELEMENT

    public:
        explicit StationListModel(QObject* parent = nullptr);
        ~StationListModel();

        enum Roles {
            Name = Qt::UserRole,
            Country,
            ImageUrl,
            Url
        };

        // Basic functionality:
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

        void setSearchQuery(QString searchQuery);
        QString searchQuery();

    signals:
        void searchQueryChanged();

    private:
        StationListModelPrivate* d;

        QCoro::Task<> setSearchQueryCore(QString searchQuery);

        // QAbstractItemModel interface
    public:
        QHash<int, QByteArray> roleNames() const;
};

#endif // STATIONLISTMODEL_H
