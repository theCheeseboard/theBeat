#include "librarymanagewidget.h"
#include "ui_librarymanagewidget.h"

#include <QListWidgetItem>
#include <QFileDialog>

LibraryManageWidget::LibraryManageWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LibraryManageWidget)
{
    ui->setupUi(this);
}

LibraryManageWidget::~LibraryManageWidget()
{
    delete ui;
}

void LibraryManageWidget::reloadLibrary() {

    ui->folders->clear();

    int count = settings.beginReadArray("library/folders");
    for (int i = 0; i < count; i++) {
        settings.setArrayIndex(i);
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(settings.value("path").toString());
        ui->folders->addItem(item);
    }
    settings.endArray();
}

void LibraryManageWidget::on_addFolderButton_clicked()
{
    QFileDialog d;
    d.setAcceptMode(QFileDialog::AcceptOpen);
    d.setFileMode(QFileDialog::DirectoryOnly);
    if (d.exec() == QFileDialog::Accepted) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(d.selectedFiles().first());
        ui->folders->addItem(item);
    }
}

void LibraryManageWidget::on_okButton_clicked()
{
    settings.beginWriteArray("library/folders");
    for (int i = 0; i < ui->folders->count(); i++) {
        settings.setArrayIndex(i);
        QListWidgetItem* item = ui->folders->item(i);
        settings.setValue("path", item->text());
    }
    settings.endArray();

    emit editingDone();
}

void LibraryManageWidget::on_removeFolderButton_clicked()
{
    for (QModelIndex item : ui->folders->selectionModel()->selectedIndexes()) {
        ui->folders->model()->removeRow(item.row());
        //ui->folders->removeItemWidget(item);
    }
}
