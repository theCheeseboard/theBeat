#include "podcastlistingwidget.h"
#include "ui_podcastlistingwidget.h"

#include "models/podcastmodel.h"
#include "podcast.h"
#include "podcastitem.h"
#include "podcastmediaitem.h"
#include <headerbackgroundcontroller.h>
#include <libcontemporary_global.h>
#include <playlist.h>
#include <sourcemanager.h>
#include <statemanager.h>
#include <ticon.h>
#include <tmessagebox.h>

struct PodcastListingWidgetPrivate {
        QPointer<Podcast> podcast;
        HeaderBackgroundController* backgroundController;
        PodcastModel* model;
};

PodcastListingWidget::PodcastListingWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::PodcastListingWidget) {
    ui->setupUi(this);
    tIcon::processWidget(this);
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
    connect(podcast, &Podcast::unsubscribed, this, [this, podcast] {
        emit done();
    });
    d->model->setPodcast(podcast);

    ui->podcastTitle->setText(podcast->name());

    QStringList podcastMeta;
    podcastMeta.append(tr("%n episodes", nullptr, d->model->rowCount()));
    ui->auxiliaryDataLabel->setText(podcastMeta.join(libContemporaryCommon::humanReadablePartJoinString()));

    d->backgroundController->setImage(QImage());
    d->backgroundController->setImage(co_await podcast->image());
    ui->menuButton->setMenu(podcast->podcastManagementMenu());
}

void PodcastListingWidget::on_backButton_clicked() {
    emit done();
}

void PodcastListingWidget::on_listView_activated(const QModelIndex& index) {
    auto podcastItem = index.data(PodcastModel::PodcastItem).value<PodcastItemPtr>();
    emit openPodcastItem(podcastItem);
}

QCoro::Task<> PodcastListingWidget::on_enqueueAllButton_clicked() {
    bool played = false;
    for (int i = d->model->rowCount() - 1; i >= 0; i--) {
        auto index = d->model->index(i, 0);
        if (index.data(PodcastModel::IsCompleted).toBool()) continue;

        auto item = new PodcastMediaItem(index.data(PodcastModel::PodcastItem).value<PodcastItemPtr>());
        StateManager::instance()->playlist()->addItem(item);
        played = true;
    }

    if (!played) {
        tMessageBox box(this->window());
        box.setTitleBarText(tr("Up to date"));
        box.setMessageText(tr("You've listened to every episode in this podcast."));
        co_await box.presentAsync();
    }
}

QCoro::Task<> PodcastListingWidget::on_playAllButton_clicked() {
    for (int i = d->model->rowCount() - 1; i >= 0; i--) {
        auto index = d->model->index(i, 0);
        if (!index.data(PodcastModel::IsCompleted).toBool()) {
            StateManager::instance()->playlist()->clear();
            ui->enqueueAllButton->click();
            co_return;
        }
    }

    tMessageBox box(this->window());
    box.setTitleBarText(tr("Up to date"));
    box.setMessageText(tr("You've listened to every episode in this podcast."));
    co_await box.presentAsync();
}

void PodcastListingWidget::on_listView_customContextMenuRequested(const QPoint& pos) {
    auto index = ui->listView->indexAt(pos);
    if (!index.isValid()) return;

    auto item = index.data(PodcastModel::PodcastItem).value<PodcastItemPtr>();
    auto menu = item->podcastManagementMenu(true);
    connect(menu, &QMenu::aboutToHide, menu, &QMenu::deleteLater);
    menu->popup(ui->listView->mapToGlobal(pos));
}
