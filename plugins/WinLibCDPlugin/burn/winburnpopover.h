#ifndef WINBURNPOPOVER_H
#define WINBURNPOPOVER_H

#include <QWidget>

namespace Ui {
    class WinBurnPopover;
}

class _bstr_t;
struct WinBurnPopoverPrivate;
class WinBurnPopover : public QWidget {
        Q_OBJECT

    public:
        explicit WinBurnPopover(QStringList files, _bstr_t driveId, QString albumName, QWidget* parent = nullptr);
        ~WinBurnPopover();

    signals:
        void done();

    private slots:
        void on_burnButton_clicked();

        void on_titleLabel_backButtonClicked();

    private:
        Ui::WinBurnPopover* ui;
        WinBurnPopoverPrivate* d;
};

#endif // WINBURNPOPOVER_H
