#ifndef MACCDPLUGINMEDIASOURCE_H
#define MACCDPLUGINMEDIASOURCE_H

#include "cdchecker.h"
#include <pluginmediasource.h>

struct MacCdPluginMediaSourcePrivate;
class MacCdPluginMediaSource : public PluginMediaSource {
        Q_PROPERTY(CdChecker* controller READ controller FINAL)
        Q_OBJECT
    public:
        explicit MacCdPluginMediaSource(CdChecker* parent = nullptr);
        ~MacCdPluginMediaSource();

        CdChecker* controller();

    signals:

    private:
        MacCdPluginMediaSourcePrivate* d;
};

#endif // MACCDPLUGINMEDIASOURCE_H
