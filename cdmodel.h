#ifndef CDMODEL_H
#define CDMODEL_H

#include <QAbstractItemModel>
#include <QDBusObjectPath>
#include <QImage>
#include <QNetworkAccessManager>

class CdModel : public QAbstractTableModel
{
        Q_OBJECT

    public:
        explicit CdModel(QString device = "sr0", QObject *parent = nullptr);

        // Header:
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

        QString cdTitle();
        QImage getArt();

    public slots:
        void eject();

    private slots:
        void checkCd();

    signals:
        void changeUiPane(int pane);
        void queryingCddb(bool querying);
        void removeAllCdTracks();

    private:
        QString device;
        QString title;
        QImage art;
        QDBusObjectPath cdDrivePath;

        QStringList trackData;
};

#endif // CDMODEL_H
