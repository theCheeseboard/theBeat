#include "macburnprovider.h"

#import <DiscRecording/DiscRecording.h>
#import <DiscRecordingUI/DiscRecordingUI.h>

#include "macburntrack.h"

struct BurnDetails {
    QStringList files;
    QString albumName;
    NSWindow* parentWindow;
};

@interface BurnPanelDelegate : NSResponder<NSWindowDelegate>
@property BurnDetails details;
@end

@implementation BurnPanelDelegate

- (id)init:(BurnDetails)burnDetails {
    self.details = burnDetails;
    return self;
}

- (void)burnSetupPanelDidEnd:(DRBurnSetupPanel*)panel returnCode:(int)returnCode contextInfo:(void*)contextInfo {
    [panel close];

    if (returnCode == NO) {
        return;
    }

    DRBurn* burn = [panel burnObject];

    NSMutableArray* tracks = [[NSMutableArray alloc] init];
    for (QString file : self.details.files) {
        MacBurnProducer* producer = [[MacBurnProducer alloc] init:file];
        MacBurnTrack* track = [[MacBurnTrack alloc] initWithProducer:producer];

        NSMutableDictionary* properties = [[NSMutableDictionary alloc] init];
        [properties setObject:[producer length] forKey:DRTrackLengthKey];
        [properties setObject:[NSNumber numberWithUnsignedShort:2352] forKey:DRBlockSizeKey];
        [properties setObject:@0 forKey:DRDataFormKey];
        [properties setObject:@0 forKey:DRBlockTypeKey];
        [properties setObject:@0 forKey:DRTrackModeKey];
        [properties setObject:@0 forKey:DRSessionFormatKey];

        [track setProperties:properties];

        [tracks addObject:track];
    }

    DRBurnProgressPanel* progress = [DRBurnProgressPanel progressPanel];
//    [progress beginProgressSheetForBurn:burn layout:tracks modalForWindow: self.details.parentWindow];
    [progress beginProgressPanelForBurn:burn layout:tracks];
}

- (BOOL)setupPanel:(DRSetupPanel*)aPanel deviceContainsSuitableMedia:(DRDevice*)device promptString:(NSString**)prompt {
    //Make sure the disc in the drive is a CD-R or CD-RW
    NSString* mediaType = [[[device status] objectForKey:DRDeviceMediaInfoKey] objectForKey:DRDeviceMediaTypeKey];
    if ([mediaType isEqualToString:DRDeviceMediaTypeCDR] == NO && [mediaType isEqualToString:DRDeviceMediaTypeCDRW] == NO) {
        *prompt = MacBurnProvider::tr("Insert a CD-R or CD-RW").toNSString();
        return NO;
    }

    return YES;
}
@end


void MacBurnProvider::burn(QStringList files, QString albumName, QWidget* parentWindow) {
    BurnDetails details;
    details.files = files;
    details.albumName = albumName;
    details.parentWindow = [reinterpret_cast<NSView*>(parentWindow->winId()) window];

    DRBurnSetupPanel* setupPanel = [DRBurnSetupPanel setupPanel];
    BurnPanelDelegate* delegate = [[BurnPanelDelegate alloc] init:details];

    [setupPanel setDelegate:delegate];
    [setupPanel beginSetupSheetForWindow:details.parentWindow modalDelegate:delegate didEndSelector:@selector(burnSetupPanelDidEnd:returnCode:contextInfo:) contextInfo:nullptr];
}
