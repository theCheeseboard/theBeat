#include "libraryresetsettingspane.h"
#include "ui_libraryresetsettingspane.h"

#include "resetlibrarypopover.h"
#include <library/librarymanager.h>
#include <tapplication.h>
#include <tpopover.h>

LibraryResetSettingsPane::LibraryResetSettingsPane(QWidget* parent) :
    tSettingsPane(parent),
    ui(new Ui::LibraryResetSettingsPane) {
    ui->setupUi(this);

    ui->resetLibraryButton->setProperty("type", "destructive");
}

LibraryResetSettingsPane::~LibraryResetSettingsPane() {
    delete ui;
}

QString LibraryResetSettingsPane::paneName() {
    return tr("Reset");
}

void LibraryResetSettingsPane::on_resetLibraryButton_clicked() {
    auto* jp = new ResetLibraryPopover();
    auto* popover = new tPopover(jp);
    popover->setPopoverWidth(SC_DPI_W(600, this));
    popover->setPopoverSide(tPopover::Trailing);
    connect(jp, &ResetLibraryPopover::done, popover, &tPopover::dismiss);
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    connect(popover, &tPopover::dismissed, jp, &ResetLibraryPopover::deleteLater);
    popover->show(this->window());
}
