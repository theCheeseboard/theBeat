#include "lastfmsettingspane.h"
#include "ui_lastfmsettingspane.h"

#include "lastfmapiservice.h"
#include "lastfmloginpopover.h"
#include <tpopover.h>
#include <tsettings.h>

struct LastFmSettingsPanePrivate {
        tSettings settings;
};

LastFmSettingsPane::LastFmSettingsPane(QWidget* parent) :
    tSettingsPane(parent),
    ui(new Ui::LastFmSettingsPane) {
    ui->setupUi(this);
    d = new LastFmSettingsPanePrivate();

    ui->logoutButton->setProperty("type", "destructive");

    connect(&d->settings, &tSettings::settingChanged, this, [this](QString key, QVariant value) {
        if (key == "lastfm/username") this->updatePane();
    });
    updatePane();
}

LastFmSettingsPane::~LastFmSettingsPane() {
    delete ui;
    delete d;
}

QString LastFmSettingsPane::paneName() {
    return tr("Account");
}

void LastFmSettingsPane::on_loginButton_clicked() {
    auto* jp = new LastFmLoginPopover();
    auto* popover = new tPopover(jp);
    popover->setPopoverWidth(SC_DPI_W(600, this));
    popover->setPopoverSide(tPopover::Trailing);
    connect(jp, &LastFmLoginPopover::done, popover, &tPopover::dismiss);
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    connect(popover, &tPopover::dismissed, jp, &LastFmLoginPopover::deleteLater);
    connect(popover, &tPopover::dismissed, this, &LastFmSettingsPane::updatePane);
    popover->show(this->window());
}

void LastFmSettingsPane::on_logoutButton_clicked() {
    LastFmApiService::logout();
    this->updatePane();
}

void LastFmSettingsPane::updatePane() {
    auto loggedInUser = LastFmApiService::loggedInUser();
    if (loggedInUser.isEmpty()) {
        ui->stackedWidget->setCurrentWidget(ui->loginPage);
    } else {
        ui->loginLabel->setText(tr("Logged in as %1").arg(loggedInUser));
        ui->stackedWidget->setCurrentWidget(ui->accountPage);
    }
}
