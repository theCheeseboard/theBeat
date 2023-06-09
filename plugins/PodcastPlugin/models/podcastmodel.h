#ifndef PODCASTMODEL_H
#define PODCASTMODEL_H

#include <QAbstractListModel>
#include <QStyledItemDelegate>
#include <tpaintcalculator.h>

class Podcast;
struct PodcastModelPrivate;
class PodcastModel : public QAbstractListModel {
        Q_OBJECT

    public:
        explicit PodcastModel(QObject* parent = nullptr);
        ~PodcastModel();

        enum Roles {
            PodcastItem = Qt::UserRole,
            Description,
            PlainDescription,
            PublishDate,
            Duration,
            PlayedDuration,
            IsCompleted
        };

        void setPodcast(Podcast* podcast);

        // Basic functionality:
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;

        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    private:
        PodcastModelPrivate* d;
};

class PodcastItemDelegate : public QStyledItemDelegate {
        Q_OBJECT
    public:
        explicit PodcastItemDelegate(QObject* parent = nullptr);

    private:
        tPaintCalculator paintCalculator(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

        // QAbstractItemDelegate interface
    public:
        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // PODCASTMODEL_H
