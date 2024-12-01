#ifndef PARANOIATRACKINFO_H
#define PARANOIATRACKINFO_H

#include <QObject>

struct ParanoiaTrackInfoPrivate;
class ParanoiaTrackInfo : public QObject {
        Q_OBJECT
    public:
        explicit ParanoiaTrackInfo();
        explicit ParanoiaTrackInfo(int track);
        ~ParanoiaTrackInfo();

        QString title();
        QStringList artist();
        QString album();
        int track();
        QImage albumArt();

        void setData(QString title, QStringList artist, QString album);
        void setAlbumArt(QImage albumArt);

    signals:
        void dataChanged();

    private:
        ParanoiaTrackInfoPrivate* d;
};

typedef QSharedPointer<ParanoiaTrackInfo> ParanoiaTrackInfoPtr;

#endif // PARANOIATRACKINFO_H
