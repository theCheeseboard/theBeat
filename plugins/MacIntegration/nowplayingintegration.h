#ifndef NOWPLAYINGINTEGRATION_H
#define NOWPLAYINGINTEGRATION_H

#include <QObject>

struct NowPlayingIntegrationPrivate;
class NowPlayingIntegration : public QObject {
        Q_OBJECT
    public:
        explicit NowPlayingIntegration(QObject* parent = nullptr);
        ~NowPlayingIntegration();

    signals:

    private:
        NowPlayingIntegrationPrivate* d;

        void updateCurrentItem();
        void updateMetadata();
};

#endif // NOWPLAYINGINTEGRATION_H
