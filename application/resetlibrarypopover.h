#ifndef RESETLIBRARYPOPOVER_H
#define RESETLIBRARYPOPOVER_H

#include <QWidget>

namespace Ui {
    class ResetLibraryPopover;
}

class ResetLibraryPopover : public QWidget {
        Q_OBJECT

    public:
        explicit ResetLibraryPopover(QWidget* parent = nullptr);
        ~ResetLibraryPopover();

    signals:
        void done();

    private slots:
        void on_resetLibraryButton_clicked();

        void on_titleLabel_backButtonClicked();

    private:
        Ui::ResetLibraryPopover* ui;
};

#endif // RESETLIBRARYPOPOVER_H
