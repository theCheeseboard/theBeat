#include "mainwindowtouchbar.h"
#include "mainwindowtouchbar_p.h"

#include <QMenu>
#include <QAction>
#include <statemanager.h>
#include <playlist.h>
#import <AppKit/AppKit.h>

@class MainWindowTouchBarProvider;

struct MainWindowTouchBarItem {
    MainWindowTouchBarItem() {};
    MainWindowTouchBarItem(QString identifier);
    MainWindowTouchBarItem(QString identifier, QAction* action);
    MainWindowTouchBarItem(QString identifier, QAction* action, NSImageName image);
    ~MainWindowTouchBarItem();

    void prepareTouchBarItem(MainWindowTouchBarProvider* provider);
    void setImage(NSImageName image);

    QString identifier;
    QAction* action = nullptr;

    bool haveImage = false;
    NSImageName image;

    NSButton* button = nil;

    NSTouchBarItem* touchBarItem = nil;
};
typedef QSharedPointer<MainWindowTouchBarItem> TouchBarItemPtr;

@interface MainWindowTouchBarProvider: NSResponder <NSTouchBarDelegate, NSApplicationDelegate, NSWindowDelegate>

@property (strong) NSCustomTouchBarItem *firstFrameItem;
@property (strong) NSCustomTouchBarItem *lastFrameItem;
@property (strong) NSButton *firstFrameButton;
@property (strong) NSButton *lastFrameButton;

@property (strong) NSObject *qtDelegate;
@property QObject *parentQObject;
@property QWidget *mainWindow;
//@property Ui::MainWindow *mainWindowUi;
@property QList<TouchBarItemPtr> touchBarActionMapping;

@end

// Create identifiers for button items.
static NSTouchBarItemIdentifier playIdentifier = @"com.vicr123.thebeat.play";
static NSTouchBarItemIdentifier skipBackIdentifier = @"com.vicr123.thebeat.skipback";
static NSTouchBarItemIdentifier skipNextIdentifier = @"com.vicr123.thebeat.skipforward";

@implementation MainWindowTouchBarProvider

- (id)init: (QWidget*)mainWin  {
    if (self = [super init]) {
        self.parentQObject = new QObject();

        //Set main window UI to call touch bar handlers
        self.mainWindow = mainWin;

        //Initialise the action mapping
        QAction* playAction = new QAction(self.parentQObject);
        playAction->setEnabled(false);
        QObject::connect(playAction, &QAction::triggered, self.parentQObject, [=] {
            StateManager::instance()->playlist()->playPause();
        });

        QAction* skipBackAction = new QAction(self.parentQObject);
        skipBackAction->setEnabled(false);
        QObject::connect(skipBackAction, &QAction::triggered, self.parentQObject, [=] {
            StateManager::instance()->playlist()->previous();
        });

        QAction* skipNextAction = new QAction(self.parentQObject);
        skipNextAction->setEnabled(false);
        QObject::connect(skipNextAction, &QAction::triggered, self.parentQObject, [=] {
            StateManager::instance()->playlist()->next();
        });

        TouchBarItemPtr playTouchBarItem(new MainWindowTouchBarItem(QString::fromNSString(playIdentifier), playAction, NSImageNameTouchBarPlayTemplate));
        QObject::connect(StateManager::instance()->playlist(), &Playlist::stateChanged, self.parentQObject, [=](Playlist::State newState, Playlist::State oldState) {
            bool enableActions = newState != Playlist::Stopped;
            playAction->setEnabled(enableActions);
            skipBackAction->setEnabled(enableActions);
            skipNextAction->setEnabled(enableActions);

            switch (newState) {
                case Playlist::Playing:
                    playTouchBarItem->setImage(NSImageNameTouchBarPauseTemplate);
                    break;
                case Playlist::Paused:
                    playTouchBarItem->setImage(NSImageNameTouchBarPlayTemplate);
                    break;
                case Playlist::Stopped:
                    playTouchBarItem->setImage(NSImageNameTouchBarPlayTemplate);
                    break;
            }
        });

        self.touchBarActionMapping = {
//            TouchBarItemPtr(new MainWindowTouchBarItem(QString::fromNSString(firstFrameIdentifier), ui->actionFirstFrame, NSImageNameTouchBarSkipToStartTemplate)),
//            TouchBarItemPtr(new MainWindowTouchBarItem(QString::fromNSString(playIdentifier), ui->actionPlay, NSImageNameTouchBarPlayTemplate)),
//            TouchBarItemPtr(new MainWindowTouchBarItem(QString::fromNSString(lastFrameIdentifier), ui->actionLastFrame, NSImageNameTouchBarSkipToEndTemplate)),
//            TouchBarItemPtr(new MainWindowTouchBarItem(QString::fromNSString(renderIdentifier), ui->actionRender, NSImageNameTouchBarRecordStartTemplate)),
//            TouchBarItemPtr(new MainWindowTouchBarItem(QString::fromNSString(inPointIdentifier), ui->actionSet_In_Point)),
//            TouchBarItemPtr(new MainWindowTouchBarItem(QString::fromNSString(outpointIdentifier), ui->actionSet_Out_Point)),
//            TouchBarItemPtr(new MainWindowTouchBarItem(QString::fromNSString(timelineIdentifier))),
//            TouchBarItemPtr(new MainWindowTouchBarItem(QString::fromNSString(timelineBarIdentifier)))
            playTouchBarItem,
            TouchBarItemPtr(new MainWindowTouchBarItem(QString::fromNSString(skipBackIdentifier), skipBackAction, NSImageNameTouchBarSkipBackTemplate)),
            TouchBarItemPtr(new MainWindowTouchBarItem(QString::fromNSString(skipNextIdentifier), skipNextAction, NSImageNameTouchBarSkipAheadTemplate))
        };

        for (TouchBarItemPtr item : self.touchBarActionMapping) {
            item->prepareTouchBarItem(self);
        }
    }

    return self;
}

- (NSTouchBar *)makeTouchBar {
    // Create the touch bar with this instance as its delegate
    NSTouchBar *bar = [[NSTouchBar alloc] init];
    bar.delegate = self;

    bar.defaultItemIdentifiers = @[skipBackIdentifier, playIdentifier, skipNextIdentifier];
    bar.customizationRequiredItemIdentifiers = @[];
//    bar.customizationAllowedItemIdentifiers = @[firstFrameIdentifier, lastFrameIdentifier, playIdentifier, inPointIdentifier, outpointIdentifier, renderIdentifier, timelineIdentifier, NSTouchBarItemIdentifierFlexibleSpace];
    [bar setCustomizationIdentifier:@"com.vicr123.thebeat.touchbar"];

    return bar;
}

- (NSTouchBarItem *)touchBar:(NSTouchBar *)touchBar makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier {
    Q_UNUSED(touchBar);

    // Create touch bar items as NSCustomTouchBarItems which can contain any NSView.
    auto result = std::find_if(self.touchBarActionMapping.constBegin(), self.touchBarActionMapping.constEnd(), [identifier](const TouchBarItemPtr& item) {
        return [identifier isEqualToString:item->identifier.toNSString()];
    });

    if (result != self.touchBarActionMapping.constEnd()) {
        return result->data()->touchBarItem;
    }
    return nil;
}

- (void)touchBarActionClicked: (NSButton*) button {
    QString identifier = QString::fromNSString([button identifier]);
    auto result = std::find_if(self.touchBarActionMapping.constBegin(), self.touchBarActionMapping.constEnd(), [identifier](const TouchBarItemPtr& item) {
        return identifier == item->identifier;
    });

    if (result != self.touchBarActionMapping.constEnd()) {
        QAction* action = result->data()->action;
        if (action->isCheckable()) {
            action->setChecked(!action->isChecked());
        } else {
            action->trigger();
        }
    }
}


- (void)installAsDelegateForWindow: (NSWindow *) window {
    _qtDelegate = window.delegate; // Save current delegate for forwarding
    window.delegate = self;
}

- (BOOL)respondsToSelector: (SEL) aSelector {
    // We want to forward to the qt delegate. Respond to selectors it
    // responds to in addition to selectors this instance resonds to.
    return [_qtDelegate respondsToSelector:aSelector] || [super respondsToSelector:aSelector];
}

- (void)forwardInvocation: (NSInvocation *) anInvocation {
    // Forward to the existing delegate. This function is only called for selectors
    // this instance does not responds to, which means that the Qt delegate
    // must respond to it (due to the respondsToSelector implementation above).
    [anInvocation invokeWithTarget:_qtDelegate];
}

- (NSApplicationPresentationOptions)window:(NSWindow *)window willUseFullScreenPresentationOptions:(NSApplicationPresentationOptions)proposedOptions {
    //On an unrelated note, set full screen window properties
    return (NSApplicationPresentationFullScreen | NSApplicationPresentationHideDock | NSApplicationPresentationAutoHideMenuBar | NSApplicationPresentationAutoHideToolbar);
}

@end


MainWindowTouchBarItem::MainWindowTouchBarItem(QString identifier) {
    this->identifier = identifier;
}

MainWindowTouchBarItem::MainWindowTouchBarItem(QString identifier, QAction* action) {
    this->action = action;
    this->identifier = identifier;
}

MainWindowTouchBarItem::MainWindowTouchBarItem(QString identifier, QAction* action, NSImageName image) {
    this->action = action;
    this->identifier = identifier;
    this->image = image;
    this->haveImage = true;
}

MainWindowTouchBarItem::~MainWindowTouchBarItem()
{
    [this->touchBarItem release];
}

void MainWindowTouchBarItem::prepareTouchBarItem(MainWindowTouchBarProvider* provider) {
//    if (identifier == QString::fromNSString(timelineIdentifier)) {
//        NSPopoverTouchBarItem* item = [[NSPopoverTouchBarItem alloc] initWithIdentifier:identifier.toNSString()];
//        [item setCustomizationLabel:QApplication::translate("MainWindow", "Timeline").toNSString()];
//        [item setCollapsedRepresentationLabel:QApplication::translate("MainWindow", "Timeline").toNSString()];
//        [item setShowsCloseButton:YES];

//        NSTouchBar* secondaryTouchBar = [[NSTouchBar alloc] init];
//        secondaryTouchBar.delegate = provider;
//        secondaryTouchBar.defaultItemIdentifiers = @[timelineBarIdentifier];
//        [item setPressAndHoldTouchBar:secondaryTouchBar];
//        [item setPopoverTouchBar:secondaryTouchBar];
//        this->touchBarItem = item;
//    } else if (identifier == QString::fromNSString(timelineBarIdentifier)) {
//        NSSliderTouchBarItem* item = [[NSSliderTouchBarItem alloc] initWithIdentifier:identifier.toNSString()];
//        [item setCustomizationLabel:QApplication::translate("MainWindow", "Timeline").toNSString()];

//        Timeline* timeline = [provider getMainWindowUi]->timeline;
//        QObject::connect(timeline, &Timeline::frameCountChanged, [=](quint64 frameCount) {
//            [item.slider setMaxValue:frameCount];
//        });
//        QObject::connect(timeline, &Timeline::currentFrameChanged, [=](quint64 currentFrame) {
//            [item.slider setDoubleValue:currentFrame];
//        });

//        [item.slider setMinValue:0];
//        [item.slider setMaxValue:timeline->frameCount()];
//        [item.slider setDoubleValue:timeline->currentFrame()];
//        [item setTarget:provider];
//        [item setAction:@selector(touchBarTimelineSliderChanged:)];

//        auto setEnabled = [=] {
//            bool enabled = true;
//            if ([provider getMainWindowUi]->stackedWidget->widget([provider getMainWindowUi]->stackedWidget->currentIndex()) != [provider getMainWindowUi]->mainPage) enabled = false;
//            [item.slider setEnabled:enabled];
//        };
//        setEnabled();
//        QObject::connect([provider getMainWindowUi]->stackedWidget, &tStackedWidget::currentChanged, setEnabled);

//        this->touchBarItem = item;
//    } else {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier.toNSString()];
        [item setCustomizationLabel:action->text().toNSString()];

        button = [NSButton buttonWithTitle:action->text().toNSString() target:provider action:@selector(touchBarActionClicked:)];
        [button setIdentifier:identifier.toNSString()];

        auto setState = [=] {
            bool enabled = true;
            if (!action->isEnabled()) enabled = false;
            if (action->menu() && !action->menu()->isEnabled()) enabled = false;
//            if ([provider getMainWindowUi]->stackedWidget->widget([provider getMainWindowUi]->stackedWidget->currentIndex()) != [provider getMainWindowUi]->mainPage) enabled = false;
            [button setEnabled:enabled];

            [button setButtonType:action->isCheckable() ? NSPushOnPushOffButton : NSMomentaryPushInButton];
            [button setState:action->isChecked() ? NSOnState : NSOffState];
        };
        setState();

        if (haveImage) {
            [button setImage:[NSImage imageNamed:image]];
        }

        item.view = button;

        QObject::connect(action, &QAction::changed, action, setState);

        this->touchBarItem = item;
        //    }
}

void MainWindowTouchBarItem::setImage(NSImageName image)
{
    this->image = image;
    [button setImage:[NSImage imageNamed:image]];
}

void MainWindowTouchBar::setupTouchBar() {
    //Disable automatic window tabbing
    [NSWindow setAllowsAutomaticWindowTabbing:NO];

    //Install TouchBarProvider as window delegate
    NSView *view = reinterpret_cast<NSView *>(d->parentWidget->winId());
    MainWindowTouchBarProvider *touchBarProvider = [[MainWindowTouchBarProvider alloc] init:d->parentWidget];
    [touchBarProvider installAsDelegateForWindow:view.window];
}
