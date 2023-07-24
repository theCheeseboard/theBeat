#ifndef LIBRARYLISTVIEW_H
#define LIBRARYLISTVIEW_H

#include <QListView>
#include <dependencyinjection/tinjectedpointer.h>
#include <iurlmanager.h>

struct LibraryListViewPrivate;
class LibraryListView : public QListView {
        Q_OBJECT
    public:
        explicit LibraryListView(QWidget* parent = nullptr, T_INJECT(IUrlManager));
        ~LibraryListView();

        void setCurrentPlaylistId(int playlistId);

    signals:

    private:
        LibraryListViewPrivate* d;

        void updatePlaylists();

        void contextMenuEvent(QContextMenuEvent* event);
};

#endif // LIBRARYLISTVIEW_H
