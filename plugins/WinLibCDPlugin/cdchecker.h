#ifndef CDCHECKER_H
#define CDCHECKER_H

#include <QWidget>
#include <QListWidgetItem>
#include <winrt/CDLib.h>

namespace Ui {
    class CdChecker;
}

struct CdCheckerPrivate;
class CdChecker : public QWidget {
        Q_OBJECT

    public:
        explicit CdChecker(QChar driveLetter, QWidget* parent = nullptr);
        ~CdChecker();



    private slots:
        void on_ejectButton_clicked();

        void on_tracksWidget_itemActivated(QListWidgetItem* item);

        void on_enqueueAllButton_clicked();

        void on_playAllButton_clicked();

        void on_shuffleAllButton_clicked();

    private:
        Ui::CdChecker* ui;
        CdCheckerPrivate* d;

        void checkCd();
        void updateTrackListing();
};

#endif // CDCHECKER_H
