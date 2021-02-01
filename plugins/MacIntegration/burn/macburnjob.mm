#include "macburnjob.h"

#include <tnotification.h>
#include "macburnjobwidget.h"

@interface MacBurnJobObserver : NSObject
@property MacBurnJob* burnJob;
@end

@implementation MacBurnJobObserver
- (id)init:(MacBurnJob*)parent {
    self.burnJob = parent;
    return self;
}

- (void)burnStatusChanged:(NSNotification*)notification {
    NSLog(@"%@", [notification userInfo]);
    self.burnJob->updateState([notification userInfo]);
}
@end

struct MacBurnJobPrivate {
    DRBurn* burn;
    QString title;
    MacBurnJobObserver* observer;

    QString state;
    double percent;
    int currentTrack;
    QString errorStatus;

    bool canceled = false;
};

MacBurnJob::MacBurnJob(void* burn, QString title, QObject* parent) : tJob(parent) {
    d = new MacBurnJobPrivate();
    d->burn = reinterpret_cast<DRBurn*>(burn);
    d->title = title;
    d->observer = [[MacBurnJobObserver alloc] init:this];
    updateState([d->burn status]);

    [[DRNotificationCenter currentRunLoopCenter] addObserver:d->observer selector:@selector(burnStatusChanged:) name:DRBurnStatusChangedNotification object:d->burn];
}

MacBurnJob::~MacBurnJob() {
    delete d;
}

void MacBurnJob::updateState(void* state) {
    NSDictionary* dict = reinterpret_cast<NSDictionary*>(state);

    d->state = QString::fromNSString([dict objectForKey:DRStatusStateKey]);
    d->percent = QString::fromNSString([[dict objectForKey:DRStatusPercentCompleteKey] stringValue]).toDouble();
    d->currentTrack = [[dict objectForKey:DRStatusCurrentTrackKey] intValue];

    NSDictionary* errorStatus = [dict objectForKey:DRErrorStatusKey];
    if (errorStatus != nil) {
        d->errorStatus = QString::fromNSString([errorStatus objectForKey:DRErrorStatusErrorStringKey]);
    }

    emit totalProgressChanged(totalProgress());
    emit progressChanged(progress());
    emit stateChanged(this->state());
    emit descriptionChanged(description());
    emit canCancelChanged(canCancel());

    //Post notifications
    if (this->state() == Finished) {
        tNotification* notification = new tNotification(tr("Burn Successful"), tr("Burned %1 to disc").arg(QLocale().quoteString(d->title)));
        notification->post();
    } else if (this->state() == Failed) {
        tNotification* notification = new tNotification(tr("Burn Failure"), tr("Failed to burn %1 to disc").arg(QLocale().quoteString(d->title)));
        notification->post();
    }
}

QString MacBurnJob::description() {
    if (d->state == QString::fromNSString(DRStatusStateDone)) {
        return tr("Burn Complete");
    } else if (d->state == QString::fromNSString(DRStatusStateFailed)) {
        if (d->canceled) {
            return tr("Burn cancelled");
        } else {
            return tr("Failed to burn: %1").arg(d->errorStatus);
        }
    } else if (d->state == QString::fromNSString(DRStatusStateErasing)) {
        return tr("Erasing disc");
    } else if (d->state == QString::fromNSString(DRStatusStateFinishing) ||
        d->state == QString::fromNSString(DRStatusStateSessionClose) ||
        d->state == QString::fromNSString(DRStatusStateTrackClose)) {
        return tr("Finalising disc");
    } else if (d->state == QString::fromNSString(DRStatusStatePreparing) ||
        d->state == QString::fromNSString(DRStatusStateSessionOpen) ||
        d->state == QString::fromNSString(DRStatusStateTrackOpen)) {
        return tr("Preparing to burn");
    } else if (d->state == QString::fromNSString(DRStatusStateSessionClose)) {
        return tr("Preparing to burn");
    } else if (d->state == QString::fromNSString(DRStatusStateTrackWrite)) {
        return tr("Burning track %1").arg(d->currentTrack);
    } else {
        return tr("Burning");
    }
}

bool MacBurnJob::canCancel() {
    return !d->canceled;
}

void MacBurnJob::cancel() {
    d->canceled = true;
    [d->burn abort];
    emit canCancelChanged(canCancel());
}

quint64 MacBurnJob::progress() {
    if (d->percent == -1) return 0;
    if (state() == Finished) return 1;
    return d->percent * 100;
}

quint64 MacBurnJob::totalProgress() {
    if (d->percent == -1) return 0;
    if (state() == Finished) return 1;
    return 100;
}

tJob::State MacBurnJob::state() {

    if (d->state == QString::fromNSString(DRStatusStateDone)) {
        return Finished;
    } else if (d->state == QString::fromNSString(DRStatusStateFailed)) {
        return Failed;
    } else {
        return Processing;
    }
}

QWidget* MacBurnJob::makeProgressWidget() {
    return new MacBurnJobWidget(this);
}
