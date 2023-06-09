#ifndef THEBEATCOMMON_H
#define THEBEATCOMMON_H

#include "libthebeat_global.h"
#include <QCoreApplication>
#include <QString>

class LIBTHEBEAT_EXPORT TheBeatCommon {
        Q_DECLARE_TR_FUNCTIONS(Common)

    public:
        static QString durationToString(quint64 ms, bool zeroIsInfinity = false);
};
#endif // THEBEATCOMMON_H
