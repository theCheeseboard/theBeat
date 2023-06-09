#include "resetlibrarypopover.h"
#include "ui_resetlibrarypopover.h"

#include <library/librarymanager.h>
#include <tapplication.h>

ResetLibraryPopover::ResetLibraryPopover(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ResetLibraryPopover) {
    ui->setupUi(this);

    ui->resetLibraryButton->setProperty("type", "destructive");
}

ResetLibraryPopover::~ResetLibraryPopover() {
    delete ui;
}

void ResetLibraryPopover::on_resetLibraryButton_clicked() {
    LibraryManager::instance()->erase();
    tApplication::restart();
}

void ResetLibraryPopover::on_titleLabel_backButtonClicked() {
    emit done();
}
