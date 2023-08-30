#ifndef LASTFMAPISERVICE_H
#define LASTFMAPISERVICE_H

#include <QCoroTask>
#include <QObject>
#include <texception.h>

struct LastFmApiServicePrivate;
class LastFmApiService : public QObject {
        Q_OBJECT
    public:
        struct GetSessionResponse {
                QString sessionKey;
                QString username;
        };

        static QString apiKey();
        static QString loggedInUser();
        static QString loggedInSessionKey();
        static void logout();
        static QCoro::Task<QString> getUnauthenticatedToken();
        static QCoro::Task<> attemptLoginWithToken(QString token);

    signals:

    private:
        explicit LastFmApiService(QObject* parent = nullptr);

        static LastFmApiService* instance();
        QCoro::Task<QJsonObject> get(QString method, QMap<QString, QString> arguments);

        LastFmApiServicePrivate* d;
};

class LastFmApiException : public QException {
        T_EXCEPTION(LastFmApiException)
    public:
        explicit LastFmApiException(int error, QString reason);

        int error();

    private:
        int _error;
};

#endif // LASTFMAPISERVICE_H
