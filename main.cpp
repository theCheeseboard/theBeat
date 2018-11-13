#include "mainwindow.h"
#include "nativeeventfilter.h"
#include <QApplication>
#include <QMainWindow>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDBusConnectionInterface>
#include <QDBusInterface>

#ifdef Q_OS_MAC
#include <CoreFoundation/CFBundle.h>
#endif

int main(int argc, char *argv[])
{
    qputenv("PHONON_BACKEND", "phonon_vlc");

    QApplication a(argc, argv);

    a.setOrganizationName("theSuite");
    a.setApplicationName("theBeat");

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    QT_TRANSLATE_NOOP("MAC_APPLICATION_MENU", "Services");
    QT_TRANSLATE_NOOP("MAC_APPLICATION_MENU", "Hide %1");
    QT_TRANSLATE_NOOP("MAC_APPLICATION_MENU", "Hide Others");
    QT_TRANSLATE_NOOP("MAC_APPLICATION_MENU", "Show All");
    QT_TRANSLATE_NOOP("MAC_APPLICATION_MENU", "Preferences...");
    QT_TRANSLATE_NOOP("MAC_APPLICATION_MENU", "About %1");
    QT_TRANSLATE_NOOP("MAC_APPLICATION_MENU", "Quit %1");

    QTranslator localTranslator;
#ifdef Q_OS_MAC
    a.setAttribute(Qt::AA_DontShowIconsInMenus, true);
    a.setQuitOnLastWindowClosed(false);

    CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef macPath = CFURLCopyFileSystemPath(appUrlRef, kCFURLPOSIXPathStyle);
    const char *pathPtr = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());

    bundlePath = QString::fromLocal8Bit(pathPtr);
    localTranslator.load(QLocale::system().name(), bundlePath + "/Contents/translations/");

    CFRelease(appUrlRef);
    CFRelease(macPath);
#endif

#ifdef Q_OS_LINUX
    localTranslator.load(QLocale::system().name(), "/usr/share/thebeat/translations");
#endif

#ifdef Q_OS_WIN
    localTranslator.load(QLocale::system().name(), a.applicationDirPath() + "\\translations");
#endif

    a.installTranslator(&localTranslator);


    //Check if theBeat is already running
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.mpris.MediaPlayer2.theBeat")) {
        //theBeat is running
        QDBusInterface interface("org.mpris.MediaPlayer2.theBeat", "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2");
        interface.call("Raise");

        //Somehow tell theBeat to enqueue the requested song
        return 0;
    }

    //Seed the random generator
    qsrand(QDateTime::currentMSecsSinceEpoch());

    MainWindow w;
    w.show();

    NativeEventFilter filter(&w);
    a.installNativeEventFilter(&filter);

    //Grab media keys
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioPlay), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioNext), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioPrev), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioStop), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);

    int retcode = a.exec();

    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioPlay), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioNext), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioPrev), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioStop), AnyModifier, QX11Info::appRootWindow());
    return retcode;
}
