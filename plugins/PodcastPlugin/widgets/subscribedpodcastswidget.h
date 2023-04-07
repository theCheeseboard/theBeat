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
        void on_listWidget_itemActivated(QListWidgetItem* item);

        void on_updateButton_clicked();

        void on_addButton_clicked();

    private:
        Ui::SubscribedPodcastsWidget* ui;
        SubscribedPodcastsWidgetPrivate* d;

        void updatePodcasts();
        QCoro::Task<> addSubscription();
};

#endif // SUBSCRIBEDPODCASTSWIDGET_H
