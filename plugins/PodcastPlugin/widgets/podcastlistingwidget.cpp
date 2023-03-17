#include "podcastlistingwidget.h"
#include "ui_podcastlistingwidget.h"

#include "models/podcastmodel.h"
#include "podcast.h"
#include "podcastitem.h"
#include <headerbackgroundcontroller.h>
#include <libcontemporary_global.h>
#include <sourcemanager.h>
#include <statemanager.h>

struct PodcastListingWidgetPrivate {
        QPointer<Podcast> podcast;
        HeaderBackgroundController* backgroundController;
        PodcastModel* model;
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

    d->model = new PodcastModel(this);
    ui->listView->setModel(d->model);
    ui->listView->setItemDelegate(new PodcastItemDelegate(this));
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
    d->model->setPodcast(podcast);

    ui->podcastTitle->setText(podcast->name());

    QStringList podcastMeta;
    podcastMeta.append(tr("%n episodes", nullptr, d->model->rowCount()));
    ui->auxiliaryDataLabel->setText(podcastMeta.join(libContemporaryCommon::humanReadablePartJoinString()));

    d->backgroundController->setImage(QImage());
    d->backgroundController->setImage(co_await podcast->image());
}

void PodcastListingWidget::on_backButton_clicked() {
    emit done();
}

void PodcastListingWidget::on_listView_activated(const QModelIndex& index) {
    auto podcastItem = index.data(PodcastModel::PodcastItem).value<PodcastItemPtr>();
    emit openPodcastItem(podcastItem);
}
