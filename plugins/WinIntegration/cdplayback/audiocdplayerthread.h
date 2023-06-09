#ifndef AUDIOCDPLAYERTHREAD_H
#define AUDIOCDPLAYERTHREAD_H

#include <QThread>
#include <winrt/CDLib.h>

class AudioCdPlayerThreadPrivate;
class AudioCdPlayerThread : public QObject {
        Q_OBJECT

    public:
        static AudioCdPlayerThread* instance();

        winrt::CDLib::IAudioCDPlayer player();

    public slots:
        void start();

    signals:
        void ready();

    private:
        explicit AudioCdPlayerThread(QObject* parent = nullptr);
        AudioCdPlayerThreadPrivate* d;

        // QThread interface
    protected:
        void run();
};

#endif // AUDIOCDPLAYERTHREAD_H
