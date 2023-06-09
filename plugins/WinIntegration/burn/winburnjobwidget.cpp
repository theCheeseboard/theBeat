#include "winburnjobwidget.h"
#include "ui_winburnjobwidget.h"

struct WinBurnJobWidgetPrivate {
    WinBurnJob* parentJob;
};

WinBurnJobWidget::WinBurnJobWidget(WinBurnJob* parentJob, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::WinBurnJobWidget) {
    ui->setupUi(this);
    d = new WinBurnJobWidgetPrivate();
    d->parentJob = parentJob;

    connect(d->parentJob, &WinBurnJob::titleChanged, ui->titleLabel, [ = ](QString text) {
        ui->titleLabel->setText(text.toUpper());
    });
    ui->titleLabel->setText(d->parentJob->title().toUpper());
    connect(d->parentJob, &WinBurnJob::descriptionChanged, ui->descriptionLabel, &QLabel::setText);
    ui->descriptionLabel->setText(d->parentJob->description());

    connect(d->parentJob, &WinBurnJob::totalProgressChanged, ui->progressBar, &QProgressBar::setMaximum);
    ui->progressBar->setMaximum(d->parentJob->totalProgress());
    connect(d->parentJob, &WinBurnJob::progressChanged, ui->progressBar, &QProgressBar::setValue);
    ui->progressBar->setValue(d->parentJob->progress());
}

WinBurnJobWidget::~WinBurnJobWidget() {
    delete d;
    delete ui;
}
