#ifndef GSTTRACKINFO_H
#define GSTTRACKINFO_H

#include <QObject>

struct GstTrackInfoPrivate;
class GstTrackInfo : public QObject {
        Q_OBJECT
    public:
        explicit GstTrackInfo();
        explicit GstTrackInfo(int track);
        ~GstTrackInfo();

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
        GstTrackInfoPrivate* d;
};

typedef QSharedPointer<GstTrackInfo> GstTrackInfoPtr;

#endif // GSTTRACKINFO_H
