#include "airplaymanager.h"

#include <QWidget>
#include <QWindow>
#include <statemanager.h>
#include <controlstripmanager.h>
#include "avplayerinstance.h"
#import <AppKit/AppKit.h>
#import <AVFoundation/AVFoundation.h>
#import <AVKit/AVKit.h>

@interface RoutePickerDelegate : NSResponder<AVRoutePickerViewDelegate>

@end

@implementation RoutePickerDelegate
@end

struct AirPlayManagerPrivate {
    API_AVAILABLE(macos(10.15)) AVRoutePickerView* picker;
    RoutePickerDelegate* delegate;
};

#include <QUrl>
AirPlayManager::AirPlayManager(QObject* parent) : QObject(parent) {
    d = new AirPlayManagerPrivate();

    if (@available(macOS 10.15, *)) {
        d->delegate = [[RoutePickerDelegate alloc] init];
        d->picker = [[AVRoutePickerView alloc] init];
        [d->picker setRoutePickerButtonBordered:NO];
        [d->picker setDelegate:d->delegate];
        [d->picker setPlayer:AvPlayerInstance::instance()];

        QWidget* widget = QWidget::createWindowContainer(QWindow::fromWinId(reinterpret_cast<WId>(d->picker)));
        widget->setFixedSize(24, 24);
        StateManager::instance()->controlStrip()->addButton(widget);
    }
}

AirPlayManager::~AirPlayManager() {
    delete d;
}
