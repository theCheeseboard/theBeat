#include "mainwindowtouchbar.h"
#include "mainwindowtouchbar_p.h"

MainWindowTouchBar::MainWindowTouchBar(QWidget *parent) : QObject(parent)
{
    d = new MainWindowTouchBarPrivate();
    d->parentWidget = parent;

    setupTouchBar();
}

MainWindowTouchBar::~MainWindowTouchBar()
{
    delete d;
}
