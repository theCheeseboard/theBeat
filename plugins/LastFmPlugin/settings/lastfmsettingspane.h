#ifndef LASTFMSETTINGSPANE_H
#define LASTFMSETTINGSPANE_H

#include <QWidget>
#include <tsettingswindow/tsettingspane.h>

namespace Ui {
    class LastFmSettingsPane;
}

struct LastFmSettingsPanePrivate;
class LastFmSettingsPane : public tSettingsPane {
        Q_OBJECT

    public:
        explicit LastFmSettingsPane(QWidget* parent = nullptr);
        ~LastFmSettingsPane();

    private:
        Ui::LastFmSettingsPane* ui;

        LastFmSettingsPanePrivate* d;

        // tSettingsPane interface
    public:
        QString paneName();
    private slots:
        void on_loginButton_clicked();
        void on_logoutButton_clicked();

        void updatePane();
};

#endif // LASTFMSETTINGSPANE_H
