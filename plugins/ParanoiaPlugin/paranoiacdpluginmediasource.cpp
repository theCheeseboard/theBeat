#include "paranoiacdpluginmediasource.h"

#include "paranoiacdcontroller.h"

using namespace Qt::Literals;

struct ParanoiaCdPluginMediaSourcePrivate {
        ParanoiaCdController* parent;
};

ParanoiaCdPluginMediaSource::ParanoiaCdPluginMediaSource(ParanoiaCdController* parent) :
    PluginMediaSource{nullptr, QUrl(u"qrc:/qt/qml/com/vicr123/thebeat/plugin/paranoia/ParanoiaCdPane.qml"_s), parent}, d{new ParanoiaCdPluginMediaSourcePrivate()} {
    d->parent = parent;
    connect(d->parent, &ParanoiaCdController::albumNameChanged, this, [this] {
        this->setName(d->parent->albumName());
    });
    this->setName(d->parent->albumName());
}

ParanoiaCdPluginMediaSource::~ParanoiaCdPluginMediaSource() {
    delete d;
}

ParanoiaCdController* ParanoiaCdPluginMediaSource::controller() {
    return d->parent;
}
