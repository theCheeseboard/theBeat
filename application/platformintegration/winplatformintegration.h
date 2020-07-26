#ifndef WINPLATFORMINTEGRATION_H
#define WINPLATFORMINTEGRATION_H

#include <QWidget>

struct WinPlatformIntegrationPrivate;
class WinPlatformIntegration : public QObject
{
    Q_OBJECT
    public:
        explicit WinPlatformIntegration(QWidget *parent);
        ~WinPlatformIntegration();

    signals:

    private:
        WinPlatformIntegrationPrivate* d;

        void updateCurrentItem();
        void updateSMTC();
};

#endif // WINPLATFORMINTEGRATION_H
