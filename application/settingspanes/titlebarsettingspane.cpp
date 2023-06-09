#include "titlebarsettingspane.h"
#include "ui_titlebarsettingspane.h"

#include <tsettings.h>

struct TitlebarSettingsPanePrivate {
        tSettings settings;
};

TitlebarSettingsPane::TitlebarSettingsPane(QWidget* parent) :
    tSettingsPane(parent),
    ui(new Ui::TitlebarSettingsPane) {
    ui->setupUi(this);

    d = new TitlebarSettingsPanePrivate();
    ui->useSsdsCheckbox->setChecked(d->settings.value("appearance/useSsds").toBool());
}

TitlebarSettingsPane::~TitlebarSettingsPane() {
    delete d;
    delete ui;
}

void TitlebarSettingsPane::on_useSsdsCheckbox_toggled(bool checked) {
    d->settings.setValue("appearance/useSsds", checked);
}

QString TitlebarSettingsPane::paneName() {
    return tr("Titlebar");
}
