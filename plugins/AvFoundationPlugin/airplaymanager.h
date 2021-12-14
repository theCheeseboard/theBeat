#ifndef AIRPLAYMANAGER_H
#define AIRPLAYMANAGER_H

#include <QObject>

struct AirPlayManagerPrivate;
class AirPlayManager : public QObject {
        Q_OBJECT
    public:
        explicit AirPlayManager(QObject* parent = nullptr);
        ~AirPlayManager();

    signals:

    private:
        AirPlayManagerPrivate* d;
};

#endif // AIRPLAYMANAGER_H
