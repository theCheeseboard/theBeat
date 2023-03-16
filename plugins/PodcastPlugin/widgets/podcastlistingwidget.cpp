#include "podcastlistingwidget.h"
#include "ui_podcastlistingwidget.h"

#include "podcast.h"
#include "podcastitem.h"

struct PodcastListingWidgetPrivate {
};

PodcastListingWidget::PodcastListingWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::PodcastListingWidget) {
    ui->setupUi(this);
    d = new PodcastListingWidgetPrivate();
}

PodcastListingWidget::~PodcastListingWidget() {
    delete ui;
    delete d;
}

void PodcastListingWidget::setCurrentPodcast(Podcast* podcast) {
    ui->podcastTitle->setText(podcast->name());

    ui->listWidget->clear();
    for (auto item : podcast->items()) {
        if (!item->playUrl().isValid()) continue;
        auto listItem = new QListWidgetItem();
        listItem->setText(item->title());
        listItem->setData(Qt::UserRole, QVariant::fromValue(item));
        ui->listWidget->addItem(listItem);
    }
}

void PodcastListingWidget::on_backButton_clicked() {
    emit done();
}

void PodcastListingWidget::on_listWidget_itemActivated(QListWidgetItem* item) {
    auto podcastItem = item->data(Qt::UserRole).value<PodcastItemPtr>();
    emit openPodcastItem(podcastItem);
}
