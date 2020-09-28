#ifndef LIBRARYLISTVIEW_H
#define LIBRARYLISTVIEW_H

#include <QListView>

struct LibraryListViewPrivate;
class LibraryListView : public QListView {
        Q_OBJECT
    public:
        explicit LibraryListView(QWidget* parent = nullptr);
        ~LibraryListView();

        void setCurrentPlaylistId(int playlistId);

    signals:

    private:
        LibraryListViewPrivate* d;

        void updatePlaylists();

        void contextMenuEvent(QContextMenuEvent* event);

};

#endif // LIBRARYLISTVIEW_H
