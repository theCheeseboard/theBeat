#include "podcastpane.h"
#include "ui_podcastpane.h"

#include "podcast.h"
#include <pluginmediasource.h>
#include <sourcemanager.h>
#include <statemanager.h>

struct PodcastPanePrivate {
        PluginMediaSource* source;
};

PodcastPane::PodcastPane(QWidget* parent) :
    AbstractLibraryBrowser(parent),
    ui(new Ui::PodcastPane) {
    ui->setupUi(this);
    d = new PodcastPanePrivate();

    d->source = new PluginMediaSource(this);
    d->source->setName(tr("Podcasts"));
    d->source->setIcon(QIcon::fromTheme("radio"));

    StateManager::instance()->sources()->addSource(d->source);
    connect(StateManager::instance()->sources(), &SourceManager::padTopChanged, this, [this](int padTop) {
        this->layout()->setContentsMargins(0, padTop, 0, 0);
    });
    this->layout()->setContentsMargins(0, StateManager::instance()->sources()->padTop(), 0, 0);

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::SlideHorizontal);

    connect(ui->podcastListingPage, &PodcastListingWidget::done, this, [this] {
        ui->stackedWidget->setCurrentWidget(ui->subscribedPage);
    });
    connect(ui->subscribedPage, &SubscribedPodcastsWidget::openPodcast, this, [this](Podcast* podcast) {
        ui->podcastListingPage->setCurrentPodcast(podcast);
        ui->stackedWidget->setCurrentWidget(ui->podcastListingPage);
    });
    connect(ui->podcastListingPage, &PodcastListingWidget::openPodcastItem, this, [this](PodcastItemPtr podcastItem) {
        ui->podcastItemPage->setPodcastItem(podcastItem);
        ui->stackedWidget->setCurrentWidget(ui->podcastItemPage);
    });
    connect(ui->podcastItemPage, &PodcastItemWidget::done, this, [this] {
        ui->stackedWidget->setCurrentWidget(ui->podcastListingPage);
    });
}

PodcastPane::~PodcastPane() {
    delete d;
    delete ui;
}

AbstractLibraryBrowser::ListInformation PodcastPane::currentListInformation() {
    return {};
}
