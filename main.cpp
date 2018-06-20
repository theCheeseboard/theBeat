#include "mainwindow.h"
#include "nativeeventfilter.h"
#include <QApplication>
#include <QMainWindow>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setOrganizationName("theSuite");
    a.setApplicationName("theBeat");

    //QMainWindow w;
    //w.show();

    MainWindow w;
    w.show();

    NativeEventFilter filter(&w);
    a.installNativeEventFilter(&filter);

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
