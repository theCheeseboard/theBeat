#ifndef AVPLAYERINSTANCE_H
#define AVPLAYERINSTANCE_H

#include <QObject>
#include <QUrl>
#import <AvFoundation/AVFoundation.h>

class AvPlayerInstance {
    public:
        explicit AvPlayerInstance();

        static AVPlayer* instance();
        static void setCurrentItem(AVPlayerItem* item);
        static void setCurrentItem(AVAsset* asset);
        static void setCurrentItem(QUrl url, NSObject* selector);

        static QUrl currentUrl();

    signals:

};

#endif // AVPLAYERINSTANCE_H
