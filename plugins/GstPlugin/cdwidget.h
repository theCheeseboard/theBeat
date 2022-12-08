#ifndef CDWIDGET_H
#define CDWIDGET_H

#include <QListWidgetItem>
#include <QWidget>
#include <abstractlibrarybrowser.h>

namespace Ui {
    class CdWidget;
}

class DiskObject;
struct CdWidgetPrivate;
class CdWidget : public AbstractLibraryBrowser {
        Q_OBJECT

    public:
        explicit CdWidget(DiskObject* disk, QWidget* parent = nullptr);
        ~CdWidget();

    private:
        Ui::CdWidget* ui;
        CdWidgetPrivate* d;

        void updateTracks();

        // AbstractLibraryBrowser interface
    public:
        ListInformation currentListInformation();

    private slots:
        void on_ejectButton_clicked();
        void on_playAllButton_clicked();
        void on_shuffleAllButton_clicked();
        void on_tracksWidget_itemActivated(QListWidgetItem* item);
        void on_enqueueAllButton_clicked();
};

#endif // CDWIDGET_H
