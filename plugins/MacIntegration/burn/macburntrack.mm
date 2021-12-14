#include "macburntrack.h"

#import <AVFoundation/AVFoundation.h>

#include <QString>
#include <QUrl>
#include <QtEndian>
#include <tlogger.h>

@implementation MacBurnTrack

- (id)initWithProducer:(MacBurnProducer*)producer {
    self = [super initWithProducer:producer];
    NSMutableDictionary* properties = [[NSMutableDictionary alloc] init];
    [properties setObject:[producer length] forKey:DRTrackLengthKey];
    [properties setObject:[NSNumber numberWithUnsignedShort:2352] forKey:DRBlockSizeKey];
    [properties setObject:@0 forKey:DRDataFormKey];
    [properties setObject:@0 forKey:DRBlockTypeKey];
    [properties setObject:@0 forKey:DRTrackModeKey];
    [properties setObject:@0 forKey:DRSessionFormatKey];

    [self setProperties:properties];

    return self;
}

@end

struct MacBurnProducerPrivate {
    QString file;
    AVAudioFile* audioFile;
};

@implementation MacBurnProducer

- (id)init:(QString)file {
    self = [super init];

    self.d = new MacBurnProducerPrivate();
    self.d->file = file;

    NSError* error;
    self.d->audioFile = [[AVAudioFile alloc] initForReading:QUrl::fromLocalFile(file).toNSURL() commonFormat:AVAudioPCMFormatInt16 interleaved:YES error:&error];
    if (error) {
        tDebug("MacBurnProducer") << "Audio file creation error";
    }

    return self;
}

- (void)dealloc {
    delete self.d;
    [super dealloc];
}

- (DRMSF*)length {
    return [DRMSF msfWithFrames:[self.d->audioFile length] * 4 / 2352];
}

- (BOOL)prepareTrack:(DRTrack*)track forBurn:(DRBurn*)burn toMedia:(NSDictionary*)mediaInfo {
    return YES;
}

- (uint32_t)produceDataForTrack:(DRTrack*)track intoBuffer:(char*)buffer length:(uint32_t)bufferLength atAddress:(uint64_t)address blockSize:(uint32_t)blockSize ioFlags:(uint32_t*)flags {
    NSError* error;
    AVAudioFormat* format = [[AVAudioFormat alloc] initWithCommonFormat:AVAudioPCMFormatInt16 sampleRate:44100 channels:2 interleaved:YES];
    AVAudioPCMBuffer* pcmBuffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:format frameCapacity:bufferLength / 4];
    [self.d->audioFile readIntoBuffer:pcmBuffer error:&error];
    if (error) {
        tDebug("MacBurnProducer") << "Audio file read error";
    }

//    uint32_t written = 0;
//    int16_t* data = [pcmBuffer int16ChannelData][0];
//    for (AVAudioFrameCount i = 0; i < [pcmBuffer frameLength] * 2; i++) {
//        int16_t leData = qToLittleEndian(data[i]);
//        reinterpret_cast<int16_t*>(buffer)[i] = leData;
//        written += sizeof(int16_t);
//    }
//    return written;

    qToLittleEndian<qint16>([pcmBuffer int16ChannelData][0], [pcmBuffer frameLength] * 2, buffer);
    return [pcmBuffer frameLength] * 2 * sizeof(int16_t);
}

- (void)cleanupTrackAfterBurn:(DRTrack*)track {

}

@end
