#ifndef WINBURNPROVIDER_H
#define WINBURNPROVIDER_H

#include <QAbstractNativeEventFilter>
#include <burnbackend.h>

class _bstr_t;
struct WinBurnProviderPrivate;
class WinBurnProvider : public BurnBackend, public QAbstractNativeEventFilter {
        Q_OBJECT
        Q_PROPERTY(bool visible READ visible NOTIFY visibleChanged FINAL)
        Q_PROPERTY(QString albumName READ albumName WRITE setAlbumName NOTIFY albumNameChanged FINAL)
        Q_PROPERTY(QString admonition READ admonition NOTIFY admonitionChanged FINAL)
        Q_PROPERTY(bool admonitionIsError READ admonitionIsError NOTIFY admonitionChanged FINAL)
        Q_PROPERTY(bool imageReady READ imageReady NOTIFY imageReadyChanged FINAL)
    public:
        explicit WinBurnProvider(_bstr_t driveId, QObject* parent = nullptr);
        ~WinBurnProvider();

        bool visible();
        Q_SCRIPTABLE void close();

        QString albumName();
        void setAlbumName(QString albumName);

        QString admonition();
        bool admonitionIsError();

        bool imageReady();

        Q_SCRIPTABLE void startBurn();

    signals:
        void visibleChanged();
        void albumNameChanged();
        void admonitionChanged();
        void imageReadyChanged();

    private:
        WinBurnProviderPrivate* d;

        void updateCd();

        // BurnBackend interface
    public:
        void burn(QStringList files, QString albumName, QWidget* parentWindow);
        QString displayName();
        QUrl qmlFile();
        QString burn(QStringList files, QString albumName, QQuickWindow* parentWindow);

        // QAbstractNativeEventFilter interface
    public:
        bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result);
};

#endif // WINBURNPROVIDER_H
