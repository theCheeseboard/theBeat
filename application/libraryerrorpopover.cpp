#include "libraryerrorpopover.h"
#include "ui_libraryerrorpopover.h"

#include <QFileDialog>

#include "library/librarymanager.h"
#include "library/librarymodel.h"

struct LibraryErrorPopoverPrivate {
    QString originalPath;
};

LibraryErrorPopover::LibraryErrorPopover(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LibraryErrorPopover)
{
    ui->setupUi(this);
    d = new LibraryErrorPopoverPrivate();

    ui->titleLabel->setBackButtonShown(true);

    ui->removeButton->setProperty("type", "destructive");
}

LibraryErrorPopover::~LibraryErrorPopover()
{
    delete d;
    delete ui;
}

void LibraryErrorPopover::setData(QModelIndex index)
{
    d->originalPath = index.data(LibraryModel::PathRole).toString();

    switch (index.data(LibraryModel::ErrorRole).value<LibraryModel::Errors>()) {
        case LibraryModel::NoError:
            break;
        case LibraryModel::PathNotFoundError:
            ui->errorDetailsLabel->setText(tr("Looks like the file has gone into hiding. If you know where it is, let us know so we can play the track."));
            break;
    }
}

void LibraryErrorPopover::on_titleLabel_backButtonClicked()
{
    emit rejected();
}

void LibraryErrorPopover::on_removeButton_clicked()
{
    LibraryManager::instance()->removeTrack(d->originalPath);
    emit rejected();
}

void LibraryErrorPopover::on_locateButton_clicked()
{
    QFileDialog* dialog = new QFileDialog(this);
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->setFileMode(QFileDialog::ExistingFile);
    connect(dialog, &QFileDialog::fileSelected, this, [ = ](QString file) {
        LibraryManager::instance()->relocateTrack(d->originalPath, file);
        emit accepted(file);
    });
    connect(dialog, &QFileDialog::finished, dialog, &QFileDialog::deleteLater);
    dialog->open();
}
