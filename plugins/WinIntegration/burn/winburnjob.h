#ifndef WINBURNJOB_H
#define WINBURNJOB_H

#include <tjob.h>
#include <QCoroTask>
#include <texception.h>

class _bstr_t;

struct IDispatch;
struct DAOBurnEvents;
struct WinBurnJobPrivate;
class WinBurnDaoImage;
typedef QSharedPointer<WinBurnDaoImage> WinBurnDaoImagePtr;

class WinBurnJobException : public tException {
        T_EXCEPTION(WinBurnJobException)
};

class WinBurnJob : public tJob {
        Q_OBJECT
    public:
        explicit WinBurnJob(WinBurnDaoImagePtr daoImage, _bstr_t driveId, QString albumTitle, QObject* parent = nullptr);
        ~WinBurnJob();

        QCoro::Task<> run();

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

        // tJob interface
    public:
        QString titleString();
        QString statusString();
};

#endif // WINBURNJOB_H
