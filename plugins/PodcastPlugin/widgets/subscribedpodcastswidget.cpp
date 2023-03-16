#include "subscribedpodcastswidget.h"
#include "ui_subscribedpodcastswidget.h"

#include "podcast.h"
#include "podcastmanager.h"
#include <ticon.h>
#include <tinputdialog.h>
#include <tmessagebox.h>

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

    auto addItem = new QListWidgetItem();
    addItem->setText(tr("Subscribe to new podcast"));
    addItem->setIcon(tIcon::fromTheme("list-add"));
    addItem->setData(Qt::UserRole, QVariant::fromValue(static_cast<Podcast*>(nullptr)));
    ui->listWidget->addItem(addItem);
}

QCoro::Task<> SubscribedPodcastsWidget::on_listWidget_itemActivated(QListWidgetItem* item) {
    auto podcast = item->data(Qt::UserRole).value<Podcast*>();
    if (podcast == nullptr) {
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
            PodcastManager::instance()->updatePodcasts();
        }
    } else {
        emit openPodcast(podcast);
    }
}