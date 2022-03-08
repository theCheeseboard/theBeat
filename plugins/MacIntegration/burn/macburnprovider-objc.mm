#include "macburnprovider.h"

#import <DiscRecording/DiscRecording.h>
#import <DiscRecordingUI/DiscRecordingUI.h>

#include <tjobmanager.h>
#include <taglib/fileref.h>
#include "macburntrack.h"
#include "macburnjob.h"

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
    NSMutableArray* cdTextInformation = [[NSMutableArray alloc] init];

    NSMutableDictionary* albumCdText = [[NSMutableDictionary alloc] init];
    [albumCdText setObject:self.details.albumName.toNSString() forKey:DRCDTextTitleKey];
    [cdTextInformation addObject:albumCdText];

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

        TagLib::FileRef tagFile(file.toStdString().data());

        NSMutableDictionary* cdText = [[NSMutableDictionary alloc] init];
        [cdText setObject:QString::fromStdString(tagFile.tag()->title().to8Bit(true)).toNSString() forKey:DRCDTextTitleKey];
        [cdText setObject:QString::fromStdString(tagFile.tag()->artist().to8Bit(true)).toNSString() forKey:DRCDTextArrangerKey];
        [cdText setObject:QString::fromStdString(tagFile.tag()->artist().to8Bit(true)).toNSString() forKey:DRCDTextComposerKey];
        [cdText setObject:QString::fromStdString(tagFile.tag()->artist().to8Bit(true)).toNSString() forKey:DRCDTextPerformerKey];
        [cdText setObject:QString::fromStdString(tagFile.tag()->artist().to8Bit(true)).toNSString() forKey:DRCDTextSongwriterKey];
        [cdTextInformation addObject:cdText];
    }

    DRCDTextBlock* cdtext = [DRCDTextBlock cdTextBlockWithLanguage:@"en" encoding:DRCDTextEncodingISOLatin1Modified];
    [cdtext setTrackDictionaries:cdTextInformation];
    bool cdTextSupported = [[[[[burn device] info] objectForKey:DRDeviceWriteCapabilitiesKey] objectForKey:DRDeviceCanWriteCDTextKey] boolValue] == YES;

    NSMutableDictionary* burnProperties = [[burn properties] mutableCopy];
    [burnProperties setObject:@NO forKey:DRBurnUnderrunProtectionKey];
    if (cdTextSupported) [burnProperties setObject:cdtext forKey:DRCDTextKey];

    MacBurnJob* job = new MacBurnJob(burn, self.details.albumName);
    tJobManager::trackJob(job);

    [burn setProperties:burnProperties];
    [burn writeLayout:tracks];
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
    [setupPanel setCanSelectTestBurn:YES];
    [setupPanel setCanSelectAppendableMedia:NO];
    [setupPanel beginSetupSheetForWindow:details.parentWindow modalDelegate:delegate didEndSelector:@selector(burnSetupPanelDidEnd:returnCode:contextInfo:) contextInfo:nullptr];
}
