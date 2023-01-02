#ifndef WINPLATFORMINTEGRATION_H
#define WINPLATFORMINTEGRATION_H

#include <QWidget>
#include <QScopedPointer>

struct SmtcIntegrationPrivate;
class SmtcIntegration : public QObject {
    Q_OBJECT
    public:
        explicit SmtcIntegration(QWidget *parent = nullptr);
        ~SmtcIntegration();

    signals:

    private:
        QScopedPointer<SmtcIntegrationPrivate> d;

        void updateCurrentItem();
        void updateSMTC();
};

#endif // WINPLATFORMINTEGRATION_H
