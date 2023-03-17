#include "podcastitemwidget.h"
#include "ui_podcastitemwidget.h"

#include "podcastmediaitem.h"
#include <QDesktopServices>
#include <headerbackgroundcontroller.h>
#include <playlist.h>
#include <sourcemanager.h>
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

    connect(StateManager::instance()->sources(), &SourceManager::padTopChanged, this, [this](int padTop) {
        d->backgroundController->setTopPadding(padTop);
    });
    d->backgroundController->setTopPadding(StateManager::instance()->sources()->padTop());

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

    if (item->isDownloading()) {
        ui->downloadButton->setEnabled(false);
        ui->downloadButton->setText(tr("Download in progress..."));
    } else {
        if (item->isDownloaded()) {
            ui->downloadButton->setText(tr("Remove Download"));
        } else {
            ui->downloadButton->setText(tr("Download"));
        }
        ui->downloadButton->setEnabled(true);
    }

    d->backgroundController->setImage(QImage());
    d->backgroundController->setImage(co_await item->image());

    ui->playButton->setVisible(!item->playUrl().isEmpty());
    ui->downloadButton->setVisible(!item->playUrl().isEmpty());
}

void PodcastItemWidget::on_backButton_clicked() {
    emit done();
}

void PodcastItemWidget::on_textBrowser_anchorClicked(const QUrl& arg1) {
    QDesktopServices::openUrl(arg1);
}

void PodcastItemWidget::on_playButton_clicked() {
    auto item = new PodcastMediaItem(d->currentItem);
    StateManager::instance()->playlist()->addItem(item);
    StateManager::instance()->playlist()->setCurrentItem(item);
}

void PodcastItemWidget::on_downloadButton_clicked() {
    if (d->currentItem->isDownloaded()) {
        d->currentItem->removeDownload();
    } else {
        d->currentItem->download(false);
    }
}
