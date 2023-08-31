#ifndef SCROBBLESERVICE_H
#define SCROBBLESERVICE_H

#include <QObject>

struct ScrobbleServicePrivate;
class ScrobbleService : public QObject {
        Q_OBJECT
    public:
        explicit ScrobbleService(QObject* parent = nullptr);
        ~ScrobbleService();

    signals:

    private:
        ScrobbleServicePrivate* d;

        void updateCurrentItem();
        void tryScrobble();
        void tryNowPlaying();
};

#endif // SCROBBLESERVICE_H
