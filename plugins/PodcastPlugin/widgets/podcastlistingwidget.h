#ifndef PODCASTLISTINGWIDGET_H
#define PODCASTLISTINGWIDGET_H

#include "podcastitem.h"
#include <QWidget>

namespace Ui {
    class PodcastListingWidget;
}

class Podcast;
struct PodcastListingWidgetPrivate;
class PodcastListingWidget : public QWidget {
        Q_OBJECT

    public:
        explicit PodcastListingWidget(QWidget* parent = nullptr);
        ~PodcastListingWidget();

        QCoro::Task<> setCurrentPodcast(Podcast* podcast);

    signals:
        void done();
        void openPodcastItem(PodcastItemPtr podcastItem);

    private slots:
        void on_backButton_clicked();

        void on_listView_activated(const QModelIndex& index);

        QCoro::Task<> on_enqueueAllButton_clicked();

        QCoro::Task<> on_playAllButton_clicked();

        void on_listView_customContextMenuRequested(const QPoint& pos);

    private:
        Ui::PodcastListingWidget* ui;
        PodcastListingWidgetPrivate* d;
};

#endif // PODCASTLISTINGWIDGET_H
