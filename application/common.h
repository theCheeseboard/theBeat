#ifndef COMMON_H
#define COMMON_H

#include <QString>

namespace Common {
    QString durationToString(quint64 ms, bool zeroIsInfinity = false);
}

#endif // COMMON_H
