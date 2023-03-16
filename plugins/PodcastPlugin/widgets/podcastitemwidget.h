#ifndef PODCASTITEMWIDGET_H
#define PODCASTITEMWIDGET_H

#include "podcastitem.h"
#include <QWidget>

namespace Ui {
    class PodcastItemWidget;
}

struct PodcastItemWidgetPrivate;
class PodcastItemWidget : public QWidget {
        Q_OBJECT

    public:
        explicit PodcastItemWidget(QWidget* parent = nullptr);
        ~PodcastItemWidget();

        void setPodcastItem(PodcastItemPtr item);

    signals:
        void done();

    private slots:
        void on_backButton_clicked();

        void on_textBrowser_anchorClicked(const QUrl& arg1);

        void on_playButton_clicked();

        void on_downloadButton_clicked();

    private:
        Ui::PodcastItemWidget* ui;
        PodcastItemWidgetPrivate* d;
};

#endif // PODCASTITEMWIDGET_H
