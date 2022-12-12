#ifndef TITLEBARSETTINGSPANE_H
#define TITLEBARSETTINGSPANE_H

#include <tsettingswindow/tsettingspane.h>

namespace Ui {
    class TitlebarSettingsPane;
}

struct TitlebarSettingsPanePrivate;
class TitlebarSettingsPane : public tSettingsPane {
        Q_OBJECT

    public:
        explicit TitlebarSettingsPane(QWidget* parent = nullptr);
        ~TitlebarSettingsPane();

    private slots:
        void on_useSsdsCheckbox_toggled(bool checked);

    private:
        Ui::TitlebarSettingsPane* ui;

        TitlebarSettingsPanePrivate* d;

        // tSettingsPane interface
    public:
        QString paneName();
};

#endif // TITLEBARSETTINGSPANE_H
