#ifndef WINBURNPROVIDER_H
#define WINBURNPROVIDER_H

#include <burnbackend.h>

class _bstr_t;
struct WinBurnProviderPrivate;
class WinBurnProvider : public BurnBackend {
        Q_OBJECT
    public:
        explicit WinBurnProvider(_bstr_t driveId, QObject* parent = nullptr);
        ~WinBurnProvider();

    signals:

    private:
        WinBurnProviderPrivate* d;

        // BurnBackend interface
    public:
        void burn(QStringList files, QString albumName, QWidget* parentWindow);
        QString displayName();
};

#endif // WINBURNPROVIDER_H
