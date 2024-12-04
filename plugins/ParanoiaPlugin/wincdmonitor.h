#ifndef WINCDMONITOR_H
#define WINCDMONITOR_H

#include <QAbstractNativeEventFilter>
#include <QObject>

struct WinCdMonitorPrivate;
class WinCdMonitor : public QObject, public QAbstractNativeEventFilter {
        Q_OBJECT
    public:
        explicit WinCdMonitor(QObject* parent = nullptr);
        ~WinCdMonitor();

    signals:

    private:
        WinCdMonitorPrivate* d;

        void updateDisks();

        // QAbstractNativeEventFilter interface
    public:
        bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result);
};

#endif // WINCDMONITOR_H
