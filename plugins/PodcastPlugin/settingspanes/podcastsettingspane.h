#ifndef PODCASTSETTINGSPANE_H
#define PODCASTSETTINGSPANE_H

#include <tsettingswindow/tsettingspane.h>

namespace Ui {
    class PodcastSettingsPane;
}

struct PodcastSettingsPanePrivate;
class PodcastSettingsPane : public tSettingsPane {
        Q_OBJECT

    public:
        explicit PodcastSettingsPane(QWidget* parent = nullptr);
        ~PodcastSettingsPane();

    private:
        Ui::PodcastSettingsPane* ui;
        PodcastSettingsPanePrivate* d;

        // tSettingsPane interface
    public:
        QString paneName();
    private slots:
        void on_autoDownloadBox_toggled(bool checked);
};

#endif // PODCASTSETTINGSPANE_H
