#ifndef WINBURNJOB_H
#define WINBURNJOB_H

#include <tjob.h>
class _bstr_t;

struct IDispatch;
struct DAOBurnEvents;
struct WinBurnJobPrivate;
class WinBurnJob : public tJob {
        Q_OBJECT
    public:
        explicit WinBurnJob(QStringList files, _bstr_t driveId, QString albumTitle, QObject* parent = nullptr);
        ~WinBurnJob();

        void run();

        QString title();
        QString description();

    signals:
        void titleChanged(QString title);
        void descriptionChanged(QString description);

    protected:
        friend DAOBurnEvents;
        void notifyUpdate(IDispatch* progress);

    private:
        WinBurnJobPrivate* d;



        // tJob interface
    public:
        quint64 progress();
        quint64 totalProgress();
        State state();
        QWidget* makeProgressWidget();
};

#endif // WINBURNJOB_H
