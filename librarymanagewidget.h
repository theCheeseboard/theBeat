#ifndef LIBRARYMANAGEWIDGET_H
#define LIBRARYMANAGEWIDGET_H

#include <QWidget>
#include <QSettings>

namespace Ui {
    class LibraryManageWidget;
}

class LibraryManageWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit LibraryManageWidget(QWidget *parent = nullptr);
        ~LibraryManageWidget();

    public slots:
        void reloadLibrary();

    private slots:
        void on_addFolderButton_clicked();

        void on_okButton_clicked();

        void on_removeFolderButton_clicked();

    signals:
        void editingDone();

    private:
        Ui::LibraryManageWidget *ui;

        QSettings settings;
};

#endif // LIBRARYMANAGEWIDGET_H
