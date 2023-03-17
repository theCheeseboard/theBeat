#include "podcastlistingwidget.h"
#include "ui_podcastlistingwidget.h"

#include "podcast.h"
#include "podcastitem.h"
#include <headerbackgroundcontroller.h>
#include <libcontemporary_global.h>
#include <sourcemanager.h>
#include <statemanager.h>

struct PodcastListingWidgetPrivate {
        QPointer<Podcast> podcast;
        HeaderBackgroundController* backgroundController;
};

PodcastListingWidget::PodcastListingWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::PodcastListingWidget) {
    ui->setupUi(this);
    d = new PodcastListingWidgetPrivate();
    d->backgroundController = new HeaderBackgroundController(ui->topWidget);

    connect(StateManager::instance()->sources(), &SourceManager::padTopChanged, this, [this](int padTop) {
        d->backgroundController->setTopPadding(padTop);
    });
    d->backgroundController->setTopPadding(StateManager::instance()->sources()->padTop());
}

PodcastListingWidget::~PodcastListingWidget() {
    delete ui;
    delete d;
}

QCoro::Task<> PodcastListingWidget::setCurrentPodcast(Podcast* podcast) {
    if (d->podcast) {
        d->podcast->disconnect(this);
    }
    d->podcast = podcast;
    connect(podcast, &Podcast::itemsUpdated, this, [this, podcast] {
        this->setCurrentPodcast(podcast);
    });

    ui->podcastTitle->setText(podcast->name());

    int itemCount = 0;
    ui->listWidget->clear();
    for (auto item : podcast->items()) {
        if (!item->playUrl().isValid()) continue;
        auto listItem = new QListWidgetItem();
        listItem->setText(item->title());
        listItem->setData(Qt::UserRole, QVariant::fromValue(item));
        ui->listWidget->addItem(listItem);
        itemCount++;
    }

    QStringList podcastMeta;
    podcastMeta.append(tr("%n episodes", nullptr, itemCount));
    ui->auxiliaryDataLabel->setText(podcastMeta.join(libContemporaryCommon::humanReadablePartJoinString()));

    d->backgroundController->setImage(QImage());
    d->backgroundController->setImage(co_await podcast->image());
}

void PodcastListingWidget::on_backButton_clicked() {
    emit done();
}

void PodcastListingWidget::on_listWidget_itemActivated(QListWidgetItem* item) {
    auto podcastItem = item->data(Qt::UserRole).value<PodcastItemPtr>();
    emit openPodcastItem(podcastItem);
}
