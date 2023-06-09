#ifndef LIBRARYRESETSETTINGSPANE_H
#define LIBRARYRESETSETTINGSPANE_H

#include <tsettingswindow/tsettingspane.h>

namespace Ui {
    class LibraryResetSettingsPane;
}

class LibraryResetSettingsPane : public tSettingsPane {
        Q_OBJECT

    public:
        explicit LibraryResetSettingsPane(QWidget* parent = nullptr);
        ~LibraryResetSettingsPane();

    private:
        Ui::LibraryResetSettingsPane* ui;

        // tSettingsPane interface
    public:
        QString paneName();
    private slots:
        void on_resetLibraryButton_clicked();
};

#endif // LIBRARYRESETSETTINGSPANE_H
