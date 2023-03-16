#include "podcastitemwidget.h"
#include "ui_podcastitemwidget.h"

#include <QDesktopServices>
#include <headerbackgroundcontroller.h>
#include <playlist.h>
#include <statemanager.h>
#include <urlmanager.h>

struct PodcastItemWidgetPrivate {
        PodcastItemPtr currentItem;
        HeaderBackgroundController* backgroundController;
};

PodcastItemWidget::PodcastItemWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::PodcastItemWidget) {
    ui->setupUi(this);
    d = new PodcastItemWidgetPrivate();
    d->backgroundController = new HeaderBackgroundController(ui->topWidget);

    ui->auxiliaryDataLabel->setVisible(false);
}

PodcastItemWidget::~PodcastItemWidget() {
    delete ui;
    delete d;
}

QCoro::Task<> PodcastItemWidget::setPodcastItem(PodcastItemPtr item) {
    if (d->currentItem) {
        d->currentItem->disconnect(this);
    }

    d->currentItem = item;
    connect(item.data(), &PodcastItem::downloadStateChanged, this, [this, item] {
        this->setPodcastItem(item);
    });

    ui->podcastTitle->setText(item->title());
    ui->textBrowser->setHtml(item->description());

    if (item->isDownloaded()) {
        ui->downloadButton->setText(tr("Remove Download"));
    } else {
        ui->downloadButton->setText(tr("Download"));
    }

    d->backgroundController->setImage(QImage());
    d->backgroundController->setImage(co_await item->image());
}

void PodcastItemWidget::on_backButton_clicked() {
    emit done();
}

void PodcastItemWidget::on_textBrowser_anchorClicked(const QUrl& arg1) {
    QDesktopServices::openUrl(arg1);
}

void PodcastItemWidget::on_playButton_clicked() {
    auto item = StateManager::instance()->url()->itemForUrl(QUrl(d->currentItem->playUrl()));
    StateManager::instance()->playlist()->addItem(item);
    StateManager::instance()->playlist()->setCurrentItem(item);
}

void PodcastItemWidget::on_downloadButton_clicked() {
    if (d->currentItem->isDownloaded()) {
        d->currentItem->removeDownload();
    } else {
        d->currentItem->download();
    }
}
