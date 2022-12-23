#include "trackscommandpalettescope.h"

#include "library/librarymanager.h"
#include "library/librarymodel.h"
#include <QUrl>
#include <mediaitem.h>
#include <playlist.h>
#include <statemanager.h>
#include <urlmanager.h>

struct TracksCommandPaletteScopePrivate {
        QSharedPointer<LibraryModel> libraryModel;
};

TracksCommandPaletteScope::TracksCommandPaletteScope(QObject* parent) :
    tCommandPaletteScope{parent} {
    d = new TracksCommandPaletteScopePrivate();
    d->libraryModel.reset(LibraryManager::instance()->allTracks());
}

TracksCommandPaletteScope::~TracksCommandPaletteScope() {
    delete d;
}

int TracksCommandPaletteScope::rowCount(const QModelIndex& parent) const {
    return d->libraryModel->rowCount();
}

QVariant TracksCommandPaletteScope::data(const QModelIndex& index, int role) const {
    return d->libraryModel->index(index.row(), index.column()).data(role);
}

QString TracksCommandPaletteScope::displayName() {
    return tr("Tracks");
}

void TracksCommandPaletteScope::filter(QString filter) {
    this->beginResetModel();
    if (filter.isEmpty()) {
        d->libraryModel.reset(LibraryManager::instance()->allTracks());
    } else {
        d->libraryModel.reset(LibraryManager::instance()->searchTracks(filter));
    }
    this->endResetModel();
}

void TracksCommandPaletteScope::activate(QModelIndex index) {
    auto modelIndex = d->libraryModel->index(index.row(), index.column());

    MediaItem* item = StateManager::instance()->url()->itemForUrl(QUrl::fromLocalFile(index.data(LibraryModel::PathRole).toString()));
    StateManager::instance()->playlist()->addItem(item);
    StateManager::instance()->playlist()->setCurrentItem(item);
}
