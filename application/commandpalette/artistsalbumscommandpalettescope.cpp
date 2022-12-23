#include "artistsalbumscommandpalettescope.h"

#include "library/librarymanager.h"
#include <QStringListModel>

struct ArtistsAlbumsCommandPaletteScopePrivate {
        QStringListModel* model;
        bool isArtists;
};

ArtistsAlbumsCommandPaletteScope::ArtistsAlbumsCommandPaletteScope(bool isArtists, QObject* parent) :
    tCommandPaletteScope{parent} {
    d = new ArtistsAlbumsCommandPaletteScopePrivate();
    d->isArtists = isArtists;

    d->model = new QStringListModel();
    connect(d->model, &QStringListModel::modelAboutToBeReset, this, &ArtistsAlbumsCommandPaletteScope::modelAboutToBeReset);
    connect(d->model, &QStringListModel::modelReset, this, &ArtistsAlbumsCommandPaletteScope::modelReset);
    connect(d->model, &QStringListModel::dataChanged, this, &ArtistsAlbumsCommandPaletteScope::dataChanged);
}

ArtistsAlbumsCommandPaletteScope::~ArtistsAlbumsCommandPaletteScope() {
    delete d;
}

int ArtistsAlbumsCommandPaletteScope::rowCount(const QModelIndex& parent) const {
    return d->model->rowCount(parent);
}

QVariant ArtistsAlbumsCommandPaletteScope::data(const QModelIndex& index, int role) const {
    return d->model->data(index, role);
}

QString ArtistsAlbumsCommandPaletteScope::displayName() {
    return d->isArtists ? tr("Artists") : tr("Albums");
}

void ArtistsAlbumsCommandPaletteScope::filter(QString filter) {
    QStringList data = d->isArtists ? LibraryManager::instance()->artists() : LibraryManager::instance()->albums();
    if (filter.isEmpty()) {
        d->model->setStringList(data);
        return;
    }

    QStringList filtered;
    auto lowerFilter = filter.toLower();
    for (const auto& item : data) {
        if (item.toLower().contains(lowerFilter)) {
            filtered.append(item);
        }
    }
    d->model->setStringList(filtered);
}

void ArtistsAlbumsCommandPaletteScope::activate(QModelIndex index) {
    emit activated(index.data(Qt::DisplayRole).toString());
}
