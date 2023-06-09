#ifndef WINBURNMANAGER_H
#define WINBURNMANAGER_H

#include <QObject>

class _bstr_t;
struct BurnManagerPrivate;
struct DiscMasterEvents;
class WinBurnManager : public QObject {
        Q_OBJECT
    public:
        explicit WinBurnManager(QObject* parent = nullptr);
        ~WinBurnManager();

    signals:

    protected:
        friend DiscMasterEvents;
        void updateBurnDevices();
        void registerBurnDevice(_bstr_t device);
        void deregisterBurnDevice(_bstr_t device);

    private:
        BurnManagerPrivate* d;

};

#endif // WINBURNMANAGER_H
