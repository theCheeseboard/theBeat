#ifndef LIBRARYMANAGEDIALOG_H
#define LIBRARYMANAGEDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QListWidgetItem>
#include <QFileDialog>

namespace Ui {
class LibraryManageDialog;
}

class LibraryManageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LibraryManageDialog(QWidget *parent = 0);
    ~LibraryManageDialog();

private slots:
    void on_addFolderButton_clicked();

    void on_okButton_clicked();

    void on_removeFolderButton_clicked();

private:
    Ui::LibraryManageDialog *ui;

    QSettings settings;
    void reject();
};

#endif // LIBRARYMANAGEDIALOG_H
