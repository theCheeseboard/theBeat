#ifndef DISKWATCHER_H
#define DISKWATCHER_H

#include <QObject>

struct DiskWatcherPrivate;
class DiskWatcher : public QObject {
        Q_OBJECT
    public:
        explicit DiskWatcher(QObject* parent = nullptr);
        ~DiskWatcher();

    signals:

    private:
        DiskWatcherPrivate* d;

        void scanDisks();

};

#endif // DISKWATCHER_H
