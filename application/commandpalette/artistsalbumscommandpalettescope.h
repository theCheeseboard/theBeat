#ifndef ARTISTSALBUMSCOMMANDPALETTESCOPE_H
#define ARTISTSALBUMSCOMMANDPALETTESCOPE_H

#include <tcommandpalette/tcommandpalettescope.h>

struct ArtistsAlbumsCommandPaletteScopePrivate;
class ArtistsAlbumsCommandPaletteScope : public tCommandPaletteScope {
        Q_OBJECT
    public:
        explicit ArtistsAlbumsCommandPaletteScope(bool isArtists, QObject* parent = nullptr);
        ~ArtistsAlbumsCommandPaletteScope();

    signals:
        void activated(QString text);

    private:
        ArtistsAlbumsCommandPaletteScopePrivate* d;

        // QAbstractItemModel interface
    public:
        int rowCount(const QModelIndex& parent) const;
        QVariant data(const QModelIndex& index, int role) const;

        // tCommandPaletteScope interface
    public:
        QString displayName();
        void filter(QString filter);
        void activate(QModelIndex index);
};

#endif // ARTISTSALBUMSCOMMANDPALETTESCOPE_H
