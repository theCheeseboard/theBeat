#ifndef LASTFMAPISERVICE_H
#define LASTFMAPISERVICE_H

#include <QCoroTask>
#include <QObject>
#include <texception.h>

class MediaItem;
struct LastFmApiServicePrivate;
class LastFmApiService : public QObject {
        Q_OBJECT
    public:
        struct GetSessionResponse {
                QString sessionKey;
                QString username;
        };

        struct Scrobble {
                Scrobble() = default;
                explicit Scrobble(QJsonObject object);
                explicit Scrobble(MediaItem* mediaItem);

                QString artist;
                QString track;
                QString timestamp;
                QString album;
                QString trackNumber;
                QString duration;

                void write(QJsonObject* object, int index) const;
        };

        static QString apiKey();
        static QString loggedInUser();
        static QString loggedInSessionKey();
        static void logout();
        static QCoro::Task<QString> getUnauthenticatedToken();
        static QCoro::Task<> attemptLoginWithToken(QString token);
        static QCoro::Task<> scrobble();

        static void pushScrobble(Scrobble scrobble);

    signals:

    private:
        explicit LastFmApiService(QObject* parent = nullptr);

        static LastFmApiService* instance();
        QCoro::Task<QJsonObject> get(QString method, QMap<QString, QString> arguments);
        QCoro::Task<QJsonObject> post(QString method, QJsonObject arguments);

        void loadPendingScrobbles();
        void savePendingScrobbles();

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
