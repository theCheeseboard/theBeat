#ifndef PODCASTLISTINGWIDGET_H
#define PODCASTLISTINGWIDGET_H

#include "podcastitem.h"
#include <QWidget>

namespace Ui {
    class PodcastListingWidget;
}

class Podcast;
class QListWidgetItem;
struct PodcastListingWidgetPrivate;
class PodcastListingWidget : public QWidget {
        Q_OBJECT

    public:
        explicit PodcastListingWidget(QWidget* parent = nullptr);
        ~PodcastListingWidget();

        void setCurrentPodcast(Podcast* podcast);

    signals:
        void done();
        void openPodcastItem(PodcastItemPtr podcastItem);

    private slots:
        void on_backButton_clicked();

        void on_listWidget_itemActivated(QListWidgetItem* item);

    private:
        Ui::PodcastListingWidget* ui;
        PodcastListingWidgetPrivate* d;
};

#endif // PODCASTLISTINGWIDGET_H
