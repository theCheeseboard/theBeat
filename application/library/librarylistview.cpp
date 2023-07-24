#include "librarylistview.h"

#include "libraryerrorpopover.h"
#include "librarymanager.h"
#include "librarymodel.h"
#include "tageditor/tageditor.h"
#include <QContextMenuEvent>
#include <QInputDialog>
#include <QMenu>
#include <QUrl>
#include <playlist.h>
#include <statemanager.h>
#include <tinputdialog.h>
#include <tpopover.h>

struct LibraryListViewPrivate {
        T_INJECTED(IUrlManager);

        QMenu* addToPlaylistOptions;
        QMenu* removeFromPlaylistMenu;
        QMenu* removeFromLibraryMenu;

        int playlistId = -1;
};

LibraryListView::LibraryListView(QWidget* parent, T_INJECTED(IUrlManager)) :
    QListView(parent) {
    d = new LibraryListViewPrivate();
    T_INJECT_SAVE_D(IUrlManager);

    d->addToPlaylistOptions = new QMenu(this);
    d->addToPlaylistOptions->setIcon(QIcon::fromTheme("list-add"));
    d->addToPlaylistOptions->setTitle(tr("Add to Playlist"));

    d->removeFromPlaylistMenu = new QMenu(this);
    d->removeFromPlaylistMenu->setIcon(QIcon::fromTheme("edit-delete"));
    d->removeFromPlaylistMenu->setTitle(tr("Remove from Playlist"));
    d->removeFromPlaylistMenu->addSection(tr("Are you sure?"));
    d->removeFromPlaylistMenu->addAction(QIcon::fromTheme("edit-delete"), tr("Remove from Playlist"), this, [this] {
        for (QModelIndex index : this->selectedIndexes()) {
            LibraryManager::instance()->removeTrackFromPlaylist(d->playlistId, index.data(LibraryModel::SortRole).toInt());
        }
        LibraryManager::instance()->normalisePlaylistSort(d->playlistId);
    });

    d->removeFromLibraryMenu = new QMenu(this);
    d->removeFromLibraryMenu->setIcon(QIcon::fromTheme("edit-delete"));
    d->removeFromLibraryMenu->setTitle(tr("Remove from Library"));
    d->removeFromLibraryMenu->addSection(tr("Are you sure?"));
    d->removeFromLibraryMenu->addAction(QIcon::fromTheme("edit-delete"), tr("Remove from Library"), this, [this] {
        for (QModelIndex index : this->selectedIndexes()) {
            LibraryManager::instance()->blacklistTrack(index.data(LibraryModel::PathRole).toString());
        }
    });

    this->setItemDelegate(new LibraryItemDelegate);

    auto activate = [this](const QModelIndex& index) {
        // Check if the user is trying to select multiple items
        if (QApplication::keyboardModifiers() & Qt::ControlModifier || QApplication::keyboardModifiers() & Qt::ShiftModifier) return;

        if (index.data(LibraryModel::ErrorRole).value<LibraryModel::Errors>() != LibraryModel::NoError) {
            LibraryErrorPopover* p = new LibraryErrorPopover(this);
            p->setData(index);

            tPopover* popover = new tPopover(p, this);
            popover->setPopoverWidth(400);
            connect(p, &LibraryErrorPopover::rejected, popover, &tPopover::dismiss);
            connect(p, &LibraryErrorPopover::accepted, popover, [=](QString newPath) {
                MediaItem* item = T_INJECTED_SERVICE(IUrlManager)->itemForUrl(QUrl::fromLocalFile(newPath));
                StateManager::instance()->playlist()->addItem(item);
                StateManager::instance()->playlist()->setCurrentItem(item);

                popover->dismiss();
            });
            connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
            connect(popover, &tPopover::dismissed, p, &LibraryErrorPopover::deleteLater);
            popover->show(this->window());
            return;
        }

        MediaItem* item = T_INJECTED_SERVICE(IUrlManager)->itemForUrl(QUrl::fromLocalFile(index.data(LibraryModel::PathRole).toString()));
        StateManager::instance()->playlist()->addItem(item);
        StateManager::instance()->playlist()->setCurrentItem(item);
    };

    connect(this, &LibraryListView::activated, this, activate);

    this->setSelectionMode(QListView::ExtendedSelection);
    this->setDragDropMode(QListView::DragDrop);
    this->setDragEnabled(true);

    connect(LibraryManager::instance(), &LibraryManager::playlistsChanged, this, &LibraryListView::updatePlaylists);
    updatePlaylists();
}

LibraryListView::~LibraryListView() {
    delete d;
}

void LibraryListView::setCurrentPlaylistId(int playlistId) {
    d->playlistId = playlistId;
}

void LibraryListView::updatePlaylists() {
    d->addToPlaylistOptions->clear();
    for (QPair<int, QString> playlist : LibraryManager::instance()->playlists()) {
        d->addToPlaylistOptions->addAction(playlist.second, this, [this, playlist] {
            for (QModelIndex index : this->selectedIndexes()) {
                LibraryManager::instance()->addTrackToPlaylist(playlist.first, index.data(LibraryModel::PathRole).toString());
            }
        });
    }

    d->addToPlaylistOptions->addSeparator();

    d->addToPlaylistOptions->addAction(QIcon::fromTheme("list-add"), tr("New Playlist"), this, [this] {
        bool ok;
        QString playlistName = tInputDialog::getText(this->window(), tr("New Playlist"), tr("What do you want to call this playlist?"), QLineEdit::Normal, "", &ok);
        if (ok) {
            int newPlaylistId = LibraryManager::instance()->createPlaylist(playlistName);
            for (QModelIndex index : this->selectedIndexes()) {
                LibraryManager::instance()->addTrackToPlaylist(newPlaylistId, index.data(LibraryModel::PathRole).toString());
            }
        }
    });
}

void LibraryListView::contextMenuEvent(QContextMenuEvent* event) {
    if (this->selectedIndexes().count() == 0) return;

    QMenu* menu = new QMenu();
    if (this->selectedIndexes().count() == 1) {
        menu->addSection(tr("For %1").arg(QLocale().quoteString(this->fontMetrics().elidedText(this->selectedIndexes().first().data().toString(), Qt::ElideMiddle, 300))));
        menu->addMenu(d->addToPlaylistOptions);
        if (d->playlistId != -1) menu->addMenu(d->removeFromPlaylistMenu);
        menu->addMenu(d->removeFromLibraryMenu);
        menu->addSeparator();
        menu->addAction(QIcon::fromTheme("document-properties"), tr("Track Properties"), this, [this] {
            TagEditor* p = new TagEditor(this->selectedIndexes().first().data(LibraryModel::PathRole).toString(), this);

            tPopover* popover = new tPopover(p, this);
            popover->setPopoverWidth(600);
            connect(p, &TagEditor::done, popover, &tPopover::dismiss);
            connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
            connect(popover, &tPopover::dismissed, p, &LibraryErrorPopover::deleteLater);
            popover->show(this->window());
        });
    } else {
        menu->addSection(tr("For %n items", nullptr, this->selectedIndexes().count()));
        menu->addMenu(d->addToPlaylistOptions);
        if (d->playlistId != -1) menu->addMenu(d->removeFromPlaylistMenu);
        menu->addMenu(d->removeFromLibraryMenu);
    }
    menu->popup(event->globalPos());
}
