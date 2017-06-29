#ifndef NATIVEEVENTFILTER_H
#define NATIVEEVENTFILTER_H

#include <QObject>
#include <QAbstractNativeEventFilter>
#include <QDebug>
#include <QX11Info>
#include <QTime>
#include "mainwindow.h"
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <X11/Xlib.h>
#include <X11/XF86keysym.h>

#undef status
#undef Bool

class NativeEventFilter : public QAbstractNativeEventFilter
{
public:
    NativeEventFilter(MainWindow *parent);

signals:

public slots:

private:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);

    QTime lastPress;
    MainWindow* parent;
};

#endif // NATIVEEVENTFILTER_H
