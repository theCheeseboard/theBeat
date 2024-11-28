#ifndef GSTCDPLUGINMEDIASOURCE_H
#define GSTCDPLUGINMEDIASOURCE_H

#include <pluginmediasource.h>

class GstCdController;
struct GstCdPluginMediaSourcePrivate;
class GstCdPluginMediaSource : public PluginMediaSource {
        Q_OBJECT
        Q_PROPERTY(GstCdController* controller READ controller FINAL)
    public:
        explicit GstCdPluginMediaSource(GstCdController* parent = nullptr);
        ~GstCdPluginMediaSource();

        GstCdController* controller();

    signals:

    private:
        GstCdPluginMediaSourcePrivate* d;
};

#endif // GSTCDPLUGINMEDIASOURCE_H
