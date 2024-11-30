#ifndef SMTCINTEGRATION_H
#define SMTCINTEGRATION_H

#include <QQuickWindow>
#include <QScopedPointer>

struct SmtcIntegrationPrivate;
class SmtcIntegration : public QObject {
        Q_OBJECT
    public:
        explicit SmtcIntegration(QQuickWindow* parent);
        ~SmtcIntegration();

    signals:

    private:
        QScopedPointer<SmtcIntegrationPrivate> d;

        void updateCurrentItem();
        void updateSMTC();
};

#endif // SMTCINTEGRATION_H
