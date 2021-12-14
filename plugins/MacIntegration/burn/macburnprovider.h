#ifndef MACBURNPROVIDER_H
#define MACBURNPROVIDER_H

#include <burnbackend.h>

class MacBurnProvider : public BurnBackend {
        Q_OBJECT
    public:
        explicit MacBurnProvider(QObject* parent = nullptr);

    signals:


        // BurnBackend interface
    public:
        void burn(QStringList files, QString albumName, QWidget* parentWindow);
        QString displayName();
};

#endif // MACBURNPROVIDER_H
