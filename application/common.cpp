#include "common.h"

#include <QStringList>

QString Common::durationToString(quint64 ms) {
    QStringList parts;

    qint64 seconds = ms / 1000 % 60;
    qint64 minutes = ms / 1000 / 60 % 60;
    qint64 hours = ms / 1000 / 60 / 60;

    if (hours > 0) parts.append(QString::number(hours));
    parts.append(QStringLiteral("%1").arg(minutes, 2, 10, QLatin1Char('0')));
    parts.append(QStringLiteral("%1").arg(seconds, 2, 10, QLatin1Char('0')));

    return parts.join(":");
}
