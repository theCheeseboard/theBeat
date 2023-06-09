#ifndef NOTIFICATIONSSETTINGSPANE_H
#define NOTIFICATIONSSETTINGSPANE_H

#include <tsettingswindow/tsettingspane.h>

namespace Ui {
    class NotificationsSettingsPane;
}

struct NotificationsSettingsPanePrivate;
class NotificationsSettingsPane : public tSettingsPane {
        Q_OBJECT

    public:
        explicit NotificationsSettingsPane(QWidget* parent = nullptr);
        ~NotificationsSettingsPane();

    private:
        Ui::NotificationsSettingsPane* ui;
        NotificationsSettingsPanePrivate* d;

        // tSettingsPane interface
    public:
        QString paneName();
    private slots:
        void on_trackChangeNotification_toggled(bool checked);
};

#endif // NOTIFICATIONSSETTINGSPANE_H
