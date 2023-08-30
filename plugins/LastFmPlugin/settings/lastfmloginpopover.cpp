#include "lastfmloginpopover.h"
#include "ui_lastfmloginpopover.h"

#include "lastfmapiservice.h"
#include <QDesktopServices>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <tpopover.h>

struct LastFmLoginPopoverPrivate {
        QString unauthenticatedToken;
        QUrl authUrl;

        QTimer* authCheckTimer;
};

LastFmLoginPopover::LastFmLoginPopover(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::LastFmLoginPopover) {
    ui->setupUi(this);
    d = new LastFmLoginPopoverPrivate();

    d->authCheckTimer = new QTimer(this);
    d->authCheckTimer->setInterval(2000);
    connect(d->authCheckTimer, &QTimer::timeout, this, &LastFmLoginPopover::checkAuth);
    connect(tPopover::popoverForPopoverWidget(this), &tPopover::dismissed, d->authCheckTimer, &QTimer::stop);

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::SlideHorizontal);
    this->acquireToken();
}

LastFmLoginPopover::~LastFmLoginPopover() {
    delete ui;
    delete d;
}

QCoro::Task<> LastFmLoginPopover::acquireToken() {
    try {
        d->unauthenticatedToken = co_await LastFmApiService::getUnauthenticatedToken();
        QUrl authUrl("http://www.last.fm/api/auth");
        authUrl.setQuery(QUrlQuery({
            {"api_key", LastFmApiService::apiKey()},
            {"token",   d->unauthenticatedToken   }
        }));

        ui->loginLabel->setText(QStringLiteral("<a href=\"%1\">%1</a>").arg(authUrl.toString()));
        d->authUrl = authUrl;
        ui->stackedWidget->setCurrentWidget(ui->loginPage);

        d->authCheckTimer->start();
    } catch (LastFmApiException& ex) {
        ui->stackedWidget->setCurrentWidget(ui->errorPage);
    }
}

QCoro::Task<> LastFmLoginPopover::checkAuth() {
    try {
        co_await LastFmApiService::attemptLoginWithToken(d->unauthenticatedToken);
        emit done();
    } catch (LastFmApiException& ex) {
        // ignore
    }
}

void LastFmLoginPopover::on_errorOkButton_clicked() {
    emit done();
}

void LastFmLoginPopover::on_titleLabel_2_backButtonClicked() {
    emit done();
}

void LastFmLoginPopover::on_titleLabel_backButtonClicked() {
    emit done();
}

void LastFmLoginPopover::on_loginButton_clicked() {
    QDesktopServices::openUrl(d->authUrl);
}

void LastFmLoginPopover::on_loginLabel_linkActivated(const QString& link) {
    QDesktopServices::openUrl(d->authUrl);
}
