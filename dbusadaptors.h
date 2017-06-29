#ifndef DBUSADAPTORS_H
#define DBUSADAPTORS_H

#include <QObject>
#include <QDBusAbstractAdaptor>
#include <QApplication>
#include <QDBusObjectPath>
#include "mainwindow.h"

class MainWindow;

/*
 * Adaptor class for interface org.mpris.MediaPlayer2
 */
class MediaPlayer2Adaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.mpris.MediaPlayer2\">\n"
"    <property access=\"read\" type=\"b\" name=\"CanQuit\"/>\n"
"    <property access=\"read\" type=\"b\" name=\"CanRaise\"/>\n"
"    <property access=\"read\" type=\"b\" name=\"HasTrackList\"/>\n"
"    <property access=\"read\" type=\"s\" name=\"Identity\"/>\n"
"    <property access=\"read\" type=\"s\" name=\"DesktopEntry\"/>\n"
"    <property access=\"read\" type=\"as\" name=\"SupportedMimeTypes\"/>\n"
"    <property access=\"read\" type=\"as\" name=\"SupportedUriSchemes\"/>\n"
"    <signal name=\"bringToFront\"/>\n"
"    <method name=\"Raise\"/>\n"
"    <method name=\"Quit\"/>\n"
"  </interface>\n"
        "")
public:
    MediaPlayer2Adaptor(MainWindow *parent);
    virtual ~MediaPlayer2Adaptor();

public: // PROPERTIES
    Q_PROPERTY(bool CanQuit READ canQuit)
    bool canQuit() const {
        return true;
    }

    Q_PROPERTY(bool CanRaise READ canRaise)
    bool canRaise() const {
        return true;
    }

    Q_PROPERTY(QString DesktopEntry READ desktopEntry)
    QString desktopEntry() const {
        return "thebeat";
    }

    Q_PROPERTY(bool HasTrackList READ hasTrackList)
    bool hasTrackList() const {
        return false;
    }

    Q_PROPERTY(QString Identity READ identity)
    QString identity() const {
        return "theBeat";
    }

    Q_PROPERTY(QStringList SupportedMimeTypes READ supportedMimeTypes)
    QStringList supportedMimeTypes() const {
        return QStringList() << "audio/mp3";
    }

    Q_PROPERTY(QStringList SupportedUriSchemes READ supportedUriSchemes)
    QStringList supportedUriSchemes() const {
        return QStringList() << "http" << "https";
    }

public Q_SLOTS: // METHODS
    void Quit() {
        QApplication::exit();
    }

    void Raise() {

    }
};

/*
 * Adaptor class for interface org.mpris.MediaPlayer2.Player
 */
class PlayerAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.mpris.MediaPlayer2.Player\">\n"
"    <property access=\"read\" type=\"s\" name=\"PlaybackStatus\"/>\n"
"    <property access=\"read\" type=\"a{sv}\" name=\"Metadata\">\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName\"/>\n"
"    </property>\n"
"    <property access=\"read\" type=\"d\" name=\"Rate\"/>\n"
"    <property access=\"read\" type=\"d\" name=\"MaximumRate\"/>\n"
"    <property access=\"read\" type=\"d\" name=\"MinimumRate\"/>\n"
"    <property access=\"read\" type=\"x\" name=\"Position\"/>\n"
"    <property access=\"read\" type=\"d\" name=\"Volume\"/>\n"
"    <property access=\"read\" type=\"b\" name=\"CanControl\"/>\n"
"    <property access=\"read\" type=\"b\" name=\"CanPlay\"/>\n"
"    <property access=\"read\" type=\"b\" name=\"CanPause\"/>\n"
"    <property access=\"read\" type=\"b\" name=\"CanGoPrevious\"/>\n"
"    <property access=\"read\" type=\"b\" name=\"CanGoNext\"/>\n"
"    <method name=\"Next\"/>\n"
"    <method name=\"Previous\"/>\n"
"    <method name=\"Pause\"/>\n"
"    <method name=\"PlayPause\"/>\n"
"    <method name=\"Stop\"/>\n"
"    <method name=\"Play\"/>\n"
"    <method type=\"ox\" name=\"SetPosition\"/>\n"
"    <method type=\"x\" name=\"Seek\"/>\n"
"    <signal name=\"Seeked\">\n"
"      <arg name=\"time\" type=\"x\" direction=\"out\"/>\n"
"      <annotation name=\"org.qtproject.QtDBus.QtTypeName.Out1\" value=\"qint64\"/>\n"
"    </signal>"
"  </interface>\n"
        "")
public:
    PlayerAdaptor(MainWindow *parent);
    virtual ~PlayerAdaptor();

public: // PROPERTIES
    Q_PROPERTY(bool CanControl READ canControl)
    bool canControl() const {
        return true;
    }

    Q_PROPERTY(bool CanGoNext READ canGoNext)
    bool canGoNext() const {
        return true;
    }

    Q_PROPERTY(bool CanGoPrevious READ canGoPrevious)
    bool canGoPrevious() const {
        return true;
    }

    Q_PROPERTY(bool CanPause READ canPause)
    bool canPause() const {
        return true;
    }

    Q_PROPERTY(bool CanPlay READ canPlay)
    bool canPlay() const {
        return true;
    }

    Q_PROPERTY(double MaximumRate READ maximumRate)
    double maximumRate() const {
        return 1;
    }

    Q_PROPERTY(QVariantMap Metadata READ metadata)
    QVariantMap metadata() const;

    Q_PROPERTY(double MinimumRate READ minimumRate)
    double minimumRate() const {
        return 1;
    }

    Q_PROPERTY(QString PlaybackStatus READ playbackStatus)
    QString playbackStatus() const;

    Q_PROPERTY(qlonglong Position READ position)
    qlonglong position() const;

    Q_PROPERTY(double Rate READ rate)
    double rate() const {
        return 1;
    }

    Q_PROPERTY(double Volume READ volume)
    double volume() const {
        return 1;
    }

public Q_SLOTS: // METHODS
    void Next();
    void Pause();
    void Play();
    void PlayPause();
    void Previous();
    void Seek(qint64 position);
    void SetPosition(QDBusObjectPath track, qlonglong position);
    void Stop();
Q_SIGNALS: // SIGNALS
    void Seeked(qint64 time);

private:
    MainWindow* mainWindow;
};

#endif // DBUSADAPTORS_H
