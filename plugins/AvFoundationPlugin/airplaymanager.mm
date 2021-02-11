#include "airplaymanager.h"

#include <QWidget>
#include <QWindow>
#include <statemanager.h>
#include <controlstripmanager.h>
#include "avplayerinstance.h"
#include <QToolButton>
#include <QBoxLayout>
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

#include <QTimer>
AirPlayManager::AirPlayManager(QObject* parent) : QObject(parent) {
    d = new AirPlayManagerPrivate();

    if (@available(macOS 10.15, *)) {
        d->delegate = [[RoutePickerDelegate alloc] init];
        d->picker = [[AVRoutePickerView alloc] init];
        [d->picker setRoutePickerButtonBordered:NO];
        [d->picker setDelegate:d->delegate];
        [d->picker setPlayer:AvPlayerInstance::instance()];

        QWidget* container = new QWidget();
        QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight);
        layout->setContentsMargins(0, 0, 0, 0);
        container->setLayout(layout);

        QToolButton btn;
        btn.setIcon(QIcon::fromTheme("document-save"));

        QWidget* widget = QWidget::createWindowContainer(QWindow::fromWinId(reinterpret_cast<WId>(d->picker)));
        widget->setFixedSize(btn.sizeHint());
        widget->setParent(container);
        widget->show();
        layout->addWidget(widget);

        StateManager::instance()->controlStrip()->addButton(container);
    }
}

AirPlayManager::~AirPlayManager() {
    delete d;
}
