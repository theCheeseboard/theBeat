#include "notificationssettingspane.h"
#include "ui_notificationssettingspane.h"

#include <tsettings.h>

struct NotificationsSettingsPanePrivate {
        tSettings settings;
};

NotificationsSettingsPane::NotificationsSettingsPane(QWidget* parent) :
    tSettingsPane(parent),
    ui(new Ui::NotificationsSettingsPane) {
    ui->setupUi(this);
    d = new NotificationsSettingsPanePrivate();
    ui->trackChangeNotification->setChecked(d->settings.value("notifications/trackChange").toBool());
}

NotificationsSettingsPane::~NotificationsSettingsPane() {
    delete d;
    delete ui;
}

QString NotificationsSettingsPane::paneName() {
    return tr("Notifications");
}

void NotificationsSettingsPane::on_trackChangeNotification_toggled(bool checked) {
    d->settings.setValue("notifications/trackChange", checked);
}
