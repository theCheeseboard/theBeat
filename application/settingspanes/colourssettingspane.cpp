#include "colourssettingspane.h"
#include "ui_colourssettingspane.h"

#include <tsettings.h>
#include <tstylemanager.h>

struct ColoursSettingsPanePrivate {
        tSettings settings;
};

ColoursSettingsPane::ColoursSettingsPane(QWidget* parent) :
    tSettingsPane(parent),
    ui(new Ui::ColoursSettingsPane) {
    ui->setupUi(this);

    d = new ColoursSettingsPanePrivate();
    if (d->settings.value("theme/mode").toString() == "light") {
        ui->lightButton->setChecked(true);
    } else {
        ui->darkButton->setChecked(true);
    }
}

ColoursSettingsPane::~ColoursSettingsPane() {
    delete d;
    delete ui;
}

QString ColoursSettingsPane::paneName() {
    return tr("Colours");
}

void ColoursSettingsPane::on_lightButton_toggled(bool checked) {
    if (checked) {
        d->settings.setValue("theme/mode", "light");
    }
}

void ColoursSettingsPane::on_darkButton_toggled(bool checked) {
    if (checked) {
        d->settings.setValue("theme/mode", "dark");
    }
}
