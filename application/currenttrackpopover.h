#ifndef CURRENTTRACKPOPOVER_H
#define CURRENTTRACKPOPOVER_H

#include <QWidget>

namespace Ui {
class CurrentTrackPopover;
}

struct CurrentTrackPopoverPrivate;
class CurrentTrackPopover : public QWidget
{
    Q_OBJECT

    public:
        explicit CurrentTrackPopover(QWidget *parent = nullptr);
        ~CurrentTrackPopover();

    private:
        Ui::CurrentTrackPopover *ui;
        CurrentTrackPopoverPrivate* d;

        void updateCurrentItem();
        void updateMetadata();

        void addMetadataTitle(QString title);
        void addMetadataEntry(QString entry, QString value);
        void clearMetadataInfo();

        void resizeEvent(QResizeEvent *event);
};

#endif // CURRENTTRACKPOPOVER_H
