#ifndef CONTROLSTRIPMANAGER_H
#define CONTROLSTRIPMANAGER_H

#include <QObject>
#include "libthebeat_global.h"

class LIBTHEBEAT_EXPORT ControlStripManager : public QObject {
        Q_OBJECT
    public:
        explicit ControlStripManager(QObject* parent = nullptr);

        void addButton(QWidget* button);

    signals:
        void buttonAdded(QWidget* button);
};

#endif // CONTROLSTRIPMANAGER_H
