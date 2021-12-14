#import <DiscRecording/DiscRecording.h>

#include <QString>
#include <QAudioDecoder>

struct MacBurnProducerPrivate;

@interface MacBurnProducer : NSObject
@property MacBurnProducerPrivate* d;

- (id)init:(QString)file;
- (void)dealloc;
- (DRMSF*)length;
@end

@interface MacBurnTrack : DRTrack
- (id)initWithProducer:(MacBurnProducer*)producer;
@end
