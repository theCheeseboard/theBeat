#include "subscribedpodcastswidget.h"
#include "ui_subscribedpodcastswidget.h"

#include "podcast.h"
#include "podcastmanager.h"

struct SubscribedPodcastsWidgetPrivate {
};

SubscribedPodcastsWidget::SubscribedPodcastsWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::SubscribedPodcastsWidget) {
    ui->setupUi(this);
    d = new SubscribedPodcastsWidgetPrivate();

    connect(PodcastManager::instance(), &PodcastManager::podcastsUpdated, this, &SubscribedPodcastsWidget::updatePodcasts);
    this->updatePodcasts();
}

SubscribedPodcastsWidget::~SubscribedPodcastsWidget() {
    delete ui;
    delete d;
}

void SubscribedPodcastsWidget::updatePodcasts() {
    ui->listWidget->clear();

    for (auto podcast : PodcastManager::instance()->podcasts()) {
        auto item = new QListWidgetItem();
        item->setText(podcast->name());
        item->setData(Qt::UserRole, QVariant::fromValue(podcast));
        ui->listWidget->addItem(item);
    }
}

void SubscribedPodcastsWidget::on_listWidget_itemActivated(QListWidgetItem* item) {
    auto podcast = item->data(Qt::UserRole).value<Podcast*>();
    emit openPodcast(podcast);
}
