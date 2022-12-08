#ifndef CDMONITOR_H
#define CDMONITOR_H

#include <QObject>

struct CdMonitorPrivate;
class CdMonitor : public QObject {
        Q_OBJECT
    public:
        explicit CdMonitor(QObject* parent = nullptr);
        ~CdMonitor();

    signals:

    private:
        CdMonitorPrivate* d;

        void updateDisks();
};

#endif // CDMONITOR_H
