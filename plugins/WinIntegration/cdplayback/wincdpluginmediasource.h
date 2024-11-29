#ifndef WINCDPLUGINMEDIASOURCE_H
#define WINCDPLUGINMEDIASOURCE_H

#include <pluginmediasource.h>

class CdChecker;
struct WinCdPluginMediaSourcePrivate;
class WinCdPluginMediaSource : public PluginMediaSource {
        Q_PROPERTY(CdChecker* controller READ controller FINAL)
        Q_OBJECT
    public:
        explicit WinCdPluginMediaSource(CdChecker* parent = nullptr);
        ~WinCdPluginMediaSource();

        CdChecker* controller();

    signals:

    private:
        WinCdPluginMediaSourcePrivate* d;
};

#endif // WINCDPLUGINMEDIASOURCE_H
