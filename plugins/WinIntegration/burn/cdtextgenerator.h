#ifndef CDTEXTGENERATOR_H
#define CDTEXTGENERATOR_H

#include <QSharedPointer>
#include <QObject>

struct CDTextGeneratorPrivate;
class CDTextGenerator : public QObject {
        Q_OBJECT
    public:
        explicit CDTextGenerator(QObject* parent = nullptr);
        ~CDTextGenerator();

        struct Track {
            QString title;
            QString performer;
            QString songwriter;
            QString composer;
            QString arranger;
        };

        void addTrack(Track trackInfo);

        QByteArray generate();

    signals:

    private:
        CDTextGeneratorPrivate* d;
};

typedef QSharedPointer<CDTextGenerator> CDTextGeneratorPtr;

#endif // CDTEXTGENERATOR_H
