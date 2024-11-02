#ifndef USERPLAYLISTMODEL_H
#define USERPLAYLISTMODEL_H

#include <QAbstractListModel>
#include <QQmlComponent>

class UserPlaylistModel : public QAbstractListModel {
        QML_ELEMENT
        Q_OBJECT

    public:
        explicit UserPlaylistModel(QObject* parent = nullptr);

        enum Roles {
            ModelRole = Qt::UserRole,
            IDRole
        };

        // Basic functionality:
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    private:
        // QAbstractItemModel interface
    public:
        QHash<int, QByteArray> roleNames() const override;
};

#endif // USERPLAYLISTMODEL_H
