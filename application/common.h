#ifndef COMMON_H
#define COMMON_H

#include <QCoreApplication>
#include <QString>
#include <QWidget>

class Common {
        Q_DECLARE_TR_FUNCTIONS(Common)

    public:
        static QString durationToString(quint64 ms, bool zeroIsInfinity = false);
        static void showBurnMenu(QStringList files, QString title, QWidget* atButton);
};

#endif // COMMON_H
