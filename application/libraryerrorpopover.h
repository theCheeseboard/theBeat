#ifndef LIBRARYERRORPOPOVER_H
#define LIBRARYERRORPOPOVER_H

#include <QWidget>

namespace Ui {
class LibraryErrorPopover;
}

struct LibraryErrorPopoverPrivate;
class LibraryErrorPopover : public QWidget
{
    Q_OBJECT

    public:
        explicit LibraryErrorPopover(QWidget *parent = nullptr);
        ~LibraryErrorPopover();

        void setData(QModelIndex index);

    private slots:
        void on_titleLabel_backButtonClicked();

        void on_removeButton_clicked();

        void on_locateButton_clicked();

signals:
        void rejected();
        void accepted(QString newFile);

    private:
        Ui::LibraryErrorPopover *ui;
        LibraryErrorPopoverPrivate* d;
};

#endif // LIBRARYERRORPOPOVER_H
