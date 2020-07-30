#include "librarylistview.h"

#include <QUrl>
#include "libraryerrorpopover.h"
#include "librarymodel.h"
#include "qtmultimedia/qtmultimediamediaitem.h"
#include <statemanager.h>
#include <playlist.h>
#include <tpopover.h>

LibraryListView::LibraryListView(QWidget* parent) : QListView(parent) {
    this->setItemDelegate(new LibraryItemDelegate);

    connect(this, &LibraryListView::activated, this, [ = ](QModelIndex index) {
        //Check if the user is trying to select multiple items
        if (QApplication::keyboardModifiers() & Qt::ControlModifier || QApplication::keyboardModifiers() & Qt::ShiftModifier) return;

        if (index.data(LibraryModel::ErrorRole).value<LibraryModel::Errors>() != LibraryModel::NoError) {
            LibraryErrorPopover* p = new LibraryErrorPopover(this);
            p->setData(index);

            tPopover* popover = new tPopover(p, this);
            popover->setPopoverWidth(SC_DPI(400));
            connect(p, &LibraryErrorPopover::rejected, popover, &tPopover::dismiss);
            connect(p, &LibraryErrorPopover::accepted, popover, [ = ](QString newPath) {
                QtMultimediaMediaItem* item = new QtMultimediaMediaItem(QUrl::fromLocalFile(newPath));
                StateManager::instance()->playlist()->addItem(item);
                StateManager::instance()->playlist()->setCurrentItem(item);

                popover->dismiss();
            });
            connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
            connect(popover, &tPopover::dismissed, p, &LibraryErrorPopover::deleteLater);
            popover->show(this->window());
            return;
        }

        QtMultimediaMediaItem* item = new QtMultimediaMediaItem(QUrl::fromLocalFile(index.data(LibraryModel::PathRole).toString()));
        StateManager::instance()->playlist()->addItem(item);
        StateManager::instance()->playlist()->setCurrentItem(item);
    });

    this->setSelectionMode(QListView::ExtendedSelection);
    this->setDragDropMode(QListView::DragDrop);
    this->setDragEnabled(true);
}
