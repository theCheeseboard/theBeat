#ifndef SUBSCRIBEDPODCASTSWIDGET_H
#define SUBSCRIBEDPODCASTSWIDGET_H

#include <QCoroTask>
#include <QWidget>

namespace Ui {
    class SubscribedPodcastsWidget;
}

class QListWidgetItem;
class Podcast;
struct SubscribedPodcastsWidgetPrivate;
class SubscribedPodcastsWidget : public QWidget {
        Q_OBJECT

    public:
        explicit SubscribedPodcastsWidget(QWidget* parent = nullptr);
        ~SubscribedPodcastsWidget();

    signals:
        void openPodcast(Podcast* podcast);

    private slots:
        QCoro::Task<> on_listWidget_itemActivated(QListWidgetItem* item);

    private:
        Ui::SubscribedPodcastsWidget* ui;
        SubscribedPodcastsWidgetPrivate* d;

        void updatePodcasts();
};

#endif // SUBSCRIBEDPODCASTSWIDGET_H
