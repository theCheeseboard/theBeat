#ifndef PODCASTPANE_H
#define PODCASTPANE_H

#include <abstractlibrarybrowser.h>

namespace Ui {
    class PodcastPane;
}

struct PodcastPanePrivate;
class PodcastPane : public AbstractLibraryBrowser {
        Q_OBJECT

    public:
        explicit PodcastPane(QWidget* parent = nullptr);
        ~PodcastPane();

    private:
        Ui::PodcastPane* ui;

        PodcastPanePrivate* d;

        // AbstractLibraryBrowser interface
    public:
        ListInformation currentListInformation();
};

#endif // PODCASTPANE_H
