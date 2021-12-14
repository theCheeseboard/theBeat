#ifndef MACBURNJOB_H
#define MACBURNJOB_H

#import <DiscRecording/DiscRecording.h>

#include <tjob.h>

struct MacBurnJobPrivate;
class MacBurnJob : public tJob {
        Q_OBJECT
    public:
        explicit MacBurnJob(void* burn, QString title, QObject* parent = nullptr);
        ~MacBurnJob();

        void updateState(void* state);

        QString description();
        bool canCancel();

        void cancel();

    signals:
        void descriptionChanged(QString description);
        void canCancelChanged(bool canCancel);


    private:
        MacBurnJobPrivate* d;

        // tJob interface
    public:
        quint64 progress();
        quint64 totalProgress();
        State state();
        QWidget* makeProgressWidget();
};

#endif // MACBURNJOB_H
