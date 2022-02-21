#ifndef BURNMANAGER_H
#define BURNMANAGER_H

#include <QObject>

struct BurnManagerPrivate;
class WinBurnManager : public QObject {
        Q_OBJECT
    public:
        explicit WinBurnManager(QObject* parent = nullptr);
        ~WinBurnManager();

    signals:

    private:
        BurnManagerPrivate* d;

        void updateBurnDevices();
};

#endif // BURNMANAGER_H
