#ifndef WINCDMEDIAITEM_H
#define WINCDMEDIAITEM_H

#include <mediaitem.h>
#include <winrt/CDLib.h>

struct WinCdMediaItemPrivate;
class WinCdMediaItem : public MediaItem {
        Q_OBJECT
    public:
        explicit WinCdMediaItem(QChar driveLetter, winrt::CDLib::IAudioCDTrack track);
        ~WinCdMediaItem();

        static void driveGone(QChar driveLetter);

    signals:

    private:
        WinCdMediaItemPrivate* d;

        // MediaItem interface
    public:
        void play();
        void pause();
        void stop();
        void seek(quint64 ms);
        quint64 elapsed();
        quint64 duration();
        QString title();
        QStringList authors();
        QString album();
        QImage albumArt();
        QVariant metadata(QString key);
};

#endif // WINCDMEDIAITEM_H
