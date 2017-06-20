#include "librarymanagedialog.h"
#include "ui_librarymanagedialog.h"

LibraryManageDialog::LibraryManageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LibraryManageDialog)
{
    ui->setupUi(this);

    int count = settings.beginReadArray("library/folders");
    for (int i = 0; i < count; i++) {
        settings.setArrayIndex(i);
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(settings.value("path").toString());
        ui->folders->addItem(item);
    }
    settings.endArray();
}

LibraryManageDialog::~LibraryManageDialog()
{
    delete ui;
}

void LibraryManageDialog::on_addFolderButton_clicked()
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

void LibraryManageDialog::on_okButton_clicked()
{
    this->close();
}

void LibraryManageDialog::reject() {
    settings.beginWriteArray("library/folders");
    for (int i = 0; i < ui->folders->count(); i++) {
        settings.setArrayIndex(i);
        QListWidgetItem* item = ui->folders->item(i);
        settings.setValue("path", item->text());
    }
    settings.endArray();

    QDialog::reject();
}

void LibraryManageDialog::on_removeFolderButton_clicked()
{
    for (QModelIndex item : ui->folders->selectionModel()->selectedIndexes()) {
        ui->folders->model()->removeRow(item.row());
        //ui->folders->removeItemWidget(item);
    }
}
