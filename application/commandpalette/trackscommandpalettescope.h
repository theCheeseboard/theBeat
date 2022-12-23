#ifndef TRACKSCOMMANDPALETTESCOPE_H
#define TRACKSCOMMANDPALETTESCOPE_H

#include <tcommandpalette/tcommandpalettescope.h>

struct TracksCommandPaletteScopePrivate;
class TracksCommandPaletteScope : public tCommandPaletteScope {
        Q_OBJECT
    public:
        explicit TracksCommandPaletteScope(QObject* parent = nullptr);
        ~TracksCommandPaletteScope();

    private:
        TracksCommandPaletteScopePrivate* d;

        // QAbstractItemModel interface
public:
        int rowCount(const QModelIndex &parent) const;
        QVariant data(const QModelIndex &index, int role) const;

        // tCommandPaletteScope interface
public:
        QString displayName();
        void filter(QString filter);
        void activate(QModelIndex index);
};

#endif // TRACKSCOMMANDPALETTESCOPE_H
