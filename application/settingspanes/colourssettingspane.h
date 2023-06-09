#ifndef COLOURSSETTINGSPANE_H
#define COLOURSSETTINGSPANE_H

#include <tsettingswindow/tsettingspane.h>

namespace Ui {
    class ColoursSettingsPane;
}

struct ColoursSettingsPanePrivate;
class ColoursSettingsPane : public tSettingsPane {
        Q_OBJECT

    public:
        explicit ColoursSettingsPane(QWidget* parent = nullptr);
        ~ColoursSettingsPane();

    private:
        Ui::ColoursSettingsPane* ui;
        ColoursSettingsPanePrivate* d;

        // tSettingsPane interface
    public:
        QString paneName();
    private slots:
        void on_lightButton_toggled(bool checked);
        void on_darkButton_toggled(bool checked);
};

#endif // COLOURSSETTINGSPANE_H
