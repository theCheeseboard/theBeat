#include "avfoundationmediaitem.h"

#include <QImage>
#include <QVariant>
#include <QUrl>
#include <QTimer>
#include <QFileInfo>
#include <QMediaMetaData>
#include <tlogger.h>
#include "avplayerinstance.h"
#import <AppKit/AppKit.h>
#import <AvFoundation/AVFoundation.h>

@interface AvFoundationResponder : NSResponder<AVAudioPlayerDelegate>
@property AvFoundationMediaItem* mediaItem;
@end

@implementation AvFoundationResponder
- (id)init:(AvFoundationMediaItem*)parent {
    if (self = [super init]) {
        self.mediaItem = parent;
    }

    return self;
}

- (void)playerDidFinishPlaying:(NSNotification*)notification {
    emit self.mediaItem->done();
}

- (void)playerFailedToPlay:(NSNotification*)notification {
    emit self.mediaItem->error();
}

@end


struct AvFoundationMediaItemPrivate {
    QUrl url;
    AvFoundationResponder* responder;
//    AVAudioPlayer* player;
//    static AVPlayer* player;
    AVAsset* asset;
    AVMetadataFormat primaryFormat;

    quint64 duration = 0;

    QTimer* durationTimer;
    QVariantMap metadata;
};

//AVPlayer* AvFoundationMediaItemPrivate::player = nil;

AvFoundationMediaItem::AvFoundationMediaItem(QUrl url) : MediaItem() {
    d = new AvFoundationMediaItemPrivate();
    d->url = url;

    d->responder = [[AvFoundationResponder alloc] init:this];

    d->durationTimer = new QTimer();
    d->durationTimer->setInterval(50);
    connect(d->durationTimer, &QTimer::timeout, this, [ = ] {
        emit durationChanged();
        emit elapsedChanged();
    });

    NSError* error;
//    d->player = [[AVAudioPlayer alloc] initWithContentsOfURL:url.toNSURL() error:&error];
//    [d->player setDelegate:d->responder];

//    if (error) {
//        tDebug("AvFoundationMediaItem") << "Playback error!";
//        tDebug("AvFoundationMediaItem") << QString::fromNSString([error localizedDescription]);
//        tDebug("AvFoundationMediaItem") << QString::fromNSString([error localizedFailureReason]);
//        QTimer::singleShot(0, this, &AvFoundationMediaItem::error);
//        return;
//    }

    tDebug("AvFoundationMediaItem") << "Created AVFoundation item for " << url.toString();

//    [d->player prepareToPlay];

    //Retrieve the item's metadata
    d->asset = [AVAsset assetWithURL:url.toNSURL()];
    d->duration = CMTimeGetSeconds([d->asset duration]) * 1000;
    [d->asset loadValuesAsynchronouslyForKeys:@[@"availableMetadataFormats"] completionHandler: ^ {
            NSError* error = nil;

            AVKeyValueStatus status = [d->asset statusOfValueForKey:@"availableMetadataFormats" error:&error];
            if (status == AVKeyValueStatusLoaded) {
                for (AVMetadataFormat format: [d->asset availableMetadataFormats]) {
                    NSArray<AVMetadataItem *> *metadata = [d->asset metadataForFormat:format];

                    auto extractOneString = [=](AVMetadataIdentifier identifier) {
                        NSArray<AVMetadataItem *> *items = [AVMetadataItem metadataItemsFromArray:metadata filteredByIdentifier:identifier];
                        AVMetadataItem *item = [items firstObject];
                        if (item) {
                            return QString::fromNSString([item stringValue]);
                        } else {
                            return QStringLiteral("");
                        }
                    };
                    auto extractStrings = [=](AVMetadataIdentifier identifier) {
                        NSArray<AVMetadataItem *> *items = [AVMetadataItem metadataItemsFromArray:metadata filteredByIdentifier:identifier];
                        QStringList strings;
                        for (AVMetadataItem *item: items) {
                            strings.append(QString::fromNSString([item stringValue]));
                        }
                        return strings;
                    };
                    auto extractOneInt = [=](AVMetadataIdentifier identifier, int defaultValue = -1) {
                        NSArray<AVMetadataItem *> *items = [AVMetadataItem metadataItemsFromArray:metadata filteredByIdentifier:identifier];
                        AVMetadataItem *item = [items firstObject];
                        if (item) {
                            return [[item numberValue] intValue];
                        } else {
                            return defaultValue;
                        }
                    };
                    auto extractImage = [=](AVMetadataIdentifier identifier) {
                        NSArray<AVMetadataItem *> *items = [AVMetadataItem metadataItemsFromArray:metadata filteredByIdentifier:identifier];
                        AVMetadataItem *item = [items firstObject];
                        if (item) {
//                        return QString::fromNSString([item stringValue]);
                            return QImage::fromData(QByteArray::fromNSData([item dataValue]));
                        } else {
                            return QImage();
                        }
                    };

                    d->metadata.insert("title", extractOneString(AVMetadataCommonIdentifierTitle));
                    d->metadata.insert("artist", extractStrings(AVMetadataCommonIdentifierArtist));
                    d->metadata.insert("album", extractOneString(AVMetadataCommonIdentifierAlbumName));
                    d->metadata.insert("track", extractOneInt(AVMetadataIdentifierID3MetadataTrackNumber, 0));
                    d->metadata.insert("albumart", extractImage(AVMetadataCommonIdentifierArtwork));

                    emit metadataChanged();
                }
            }
    }];
}

AvFoundationMediaItem::~AvFoundationMediaItem() {
    delete d;
}

void AvFoundationMediaItem::play() {
    AvPlayerInstance::setCurrentItem(d->url, d->responder);
    [AvPlayerInstance::instance() play];
    d->durationTimer->start();
}

void AvFoundationMediaItem::pause() {
    d->durationTimer->stop();
    [AvPlayerInstance::instance() pause];
}

void AvFoundationMediaItem::stop() {
    d->durationTimer->stop();
    [AvPlayerInstance::instance() pause];
}

void AvFoundationMediaItem::seek(quint64 ms) {
    [AvPlayerInstance::instance() seekToTime:CMTimeMake(ms, 1000)];
}

quint64 AvFoundationMediaItem::elapsed() {
    return CMTimeGetSeconds([AvPlayerInstance::instance() currentTime]) * 1000;
}

quint64 AvFoundationMediaItem::duration() {
    return d->duration;
}

QString AvFoundationMediaItem::title() {
    if (!d->metadata.value("title").toString().isEmpty()) {
        return d->metadata.value("title").toString();
    }

    if (d->url.isLocalFile()) {
        QFileInfo file(d->url.toLocalFile());
        return file.baseName();
    } else {
        return d->url.toString();
    }
}

QStringList AvFoundationMediaItem::authors() {
    return d->metadata.value("artist").toStringList();
}

QString AvFoundationMediaItem::album() {
    return d->metadata.value("album").toString();
}

QImage AvFoundationMediaItem::albumArt() {
    return d->metadata.value("albumart").value<QImage>();
}

QVariant AvFoundationMediaItem::metadata(QMediaMetaData::Key key) {
    if (key == QMediaMetaData::AlbumTitle) {
        return album();
    } else if (key == QMediaMetaData::Title) {
        return title();
    } else if (key == QMediaMetaData::Author) {
        return authors();
    } else if (key == QMediaMetaData::TrackNumber) {
        return d->metadata.value("track");
    }
    return QVariant();
}

QString AvFoundationMediaItem::lyrics() {
    //TODO
    return "";
}

QString AvFoundationMediaItem::lyricFormat()
{
    //TODO
    return "";
}
