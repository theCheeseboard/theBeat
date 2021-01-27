#ifndef CURRENTTRACKPOPOVER_H
#define CURRENTTRACKPOPOVER_H

#include <QWidget>

namespace Ui {
    class CurrentTrackPopover;
}

struct CurrentTrackPopoverPrivate;
class CurrentTrackPopover : public QWidget {
        Q_OBJECT

    public:
        explicit CurrentTrackPopover(QWidget* parent = nullptr);
        ~CurrentTrackPopover();

    private slots:
        void on_skipBackButton_clicked();

        void on_playButton_clicked();

        void on_skipNextButton_clicked();

        void on_progressSlider_valueChanged(int value);

    private:
        Ui::CurrentTrackPopover* ui;
        CurrentTrackPopoverPrivate* d;

        void updateCurrentItem();
        void updateMetadata();

        void addMetadataTitle(QString title);
        void addMetadataEntry(QString entry, QString value);
        void clearMetadataInfo();

        void updateState();
        void updateBar();

        void resizeEvent(QResizeEvent* event);
        void paintEvent(QPaintEvent* event);
};

#endif // CURRENTTRACKPOPOVER_H
