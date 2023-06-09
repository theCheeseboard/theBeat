#include "podcastsettingspane.h"
#include "ui_podcastsettingspane.h"

#include <tsettings.h>

struct PodcastSettingsPanePrivate {
        tSettings settings;
};

PodcastSettingsPane::PodcastSettingsPane(QWidget* parent) :
    tSettingsPane(parent),
    ui(new Ui::PodcastSettingsPane) {
    ui->setupUi(this);
    d = new PodcastSettingsPanePrivate();

    ui->autoDownloadBox->setChecked(d->settings.value("podcasts/autoDownload").toBool());
}

PodcastSettingsPane::~PodcastSettingsPane() {
    delete ui;
    delete d;
}

QString PodcastSettingsPane::paneName() {
    return tr("Podcasts");
}

void PodcastSettingsPane::on_autoDownloadBox_toggled(bool checked) {
    d->settings.setValue("podcasts/autoDownload", checked);
}
