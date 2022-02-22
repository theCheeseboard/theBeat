#ifndef WINBURNPOPOVER_H
#define WINBURNPOPOVER_H

#include <QWidget>
#include <QAbstractNativeEventFilter>

namespace Ui {
    class WinBurnPopover;
}

class _bstr_t;
struct WinBurnPopoverPrivate;
class WinBurnPopover : public QWidget, public QAbstractNativeEventFilter {
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

        void updateCd();

        void setOkState();
        void setWarningState(QString warning);
        void setErrorState(QString error);

        // QAbstractNativeEventFilter interface
    public:
        bool nativeEventFilter(const QByteArray& eventType, void* message, long* result);
};

#endif // WINBURNPOPOVER_H
