#include "subscribedpodcastswidget.h"
#include "ui_subscribedpodcastswidget.h"

#include "podcast.h"
#include "podcastmanager.h"
#include <sourcemanager.h>
#include <statemanager.h>
#include <ticon.h>
#include <tinputdialog.h>
#include <tmessagebox.h>

struct SubscribedPodcastsWidgetPrivate {
};

SubscribedPodcastsWidget::SubscribedPodcastsWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::SubscribedPodcastsWidget) {
    ui->setupUi(this);
    tIcon::processWidget(this);
    d = new SubscribedPodcastsWidgetPrivate();

    connect(StateManager::instance()->sources(), &SourceManager::padTopChanged, this, [this](int padTop) {
        this->layout()->setContentsMargins(0, padTop, 0, 0);
    });
    this->layout()->setContentsMargins(0, StateManager::instance()->sources()->padTop(), 0, 0);

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

    auto addItem = new QListWidgetItem();
    addItem->setText(tr("Subscribe to new podcast"));
    addItem->setIcon(tIcon::fromTheme("list-add"));
    addItem->setData(Qt::UserRole, QVariant::fromValue(static_cast<Podcast*>(nullptr)));
    ui->listWidget->addItem(addItem);
}

QCoro::Task<> SubscribedPodcastsWidget::addSubscription() {
    bool ok;
    auto urlString = tInputDialog::getText(this->window(), tr("Subscribe to new podcast"), tr("What's the address of the feed?"), QLineEdit::Normal, "", &ok);
    if (ok) {
        QUrl url(urlString);
        if (!url.isValid() || (url.scheme() != "https" && url.scheme() != "http")) {
            tMessageBox box(this->window());
            box.setTitleBarText(tr("Invalid URL"));
            box.setMessageText(tr("Sorry, that URL is not a valid feed URL"));
            box.setIcon(QMessageBox::Critical);
            co_await box.presentAsync();
            co_return;
        }
        emit openPodcast(PodcastManager::instance()->subscribe(url));
        PodcastManager::instance()->updatePodcasts(true);
    }
}

void SubscribedPodcastsWidget::on_listWidget_itemActivated(QListWidgetItem* item) {
    auto podcast = item->data(Qt::UserRole).value<Podcast*>();
    if (podcast == nullptr) {
        addSubscription();
    } else {
        emit openPodcast(podcast);
    }
}

void SubscribedPodcastsWidget::on_updateButton_clicked() {
    PodcastManager::instance()->updatePodcasts(false);
}

void SubscribedPodcastsWidget::on_addButton_clicked() {
    addSubscription();
}
