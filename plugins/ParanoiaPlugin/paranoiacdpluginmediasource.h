#ifndef PARANOIACDPLUGINMEDIASOURCE_H
#define PARANOIACDPLUGINMEDIASOURCE_H

#include <pluginmediasource.h>

class ParanoiaCdController;
struct ParanoiaCdPluginMediaSourcePrivate;
class ParanoiaCdPluginMediaSource : public PluginMediaSource {
        Q_OBJECT
        Q_PROPERTY(ParanoiaCdController* controller READ controller FINAL)
    public:
        explicit ParanoiaCdPluginMediaSource(ParanoiaCdController* parent = nullptr);
        ~ParanoiaCdPluginMediaSource();

        ParanoiaCdController* controller();

    signals:

    private:
        ParanoiaCdPluginMediaSourcePrivate* d;
};

#endif // PARANOIACDPLUGINMEDIASOURCE_H
