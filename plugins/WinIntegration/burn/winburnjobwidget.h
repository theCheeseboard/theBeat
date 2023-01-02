#ifndef WINBURNJOBWIDGET_H
#define WINBURNJOBWIDGET_H

#include <QWidget>
#include "winburnjob.h"

namespace Ui {
    class WinBurnJobWidget;
}

struct WinBurnJobWidgetPrivate;
class WinBurnJobWidget : public QWidget {
        Q_OBJECT

    public:
        explicit WinBurnJobWidget(WinBurnJob* parentJob, QWidget* parent = nullptr);
        ~WinBurnJobWidget();

    private:
        Ui::WinBurnJobWidget* ui;
        WinBurnJobWidgetPrivate* d;
};

#endif // WINBURNJOBWIDGET_H
