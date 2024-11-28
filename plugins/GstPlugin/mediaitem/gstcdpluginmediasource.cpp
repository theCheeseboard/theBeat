#include "gstcdpluginmediasource.h"

#include "../gstcdcontroller.h"

using namespace Qt::Literals;

struct GstCdPluginMediaSourcePrivate {
        GstCdController* parent;
};

GstCdPluginMediaSource::GstCdPluginMediaSource(GstCdController* parent) :
    PluginMediaSource{nullptr, QUrl(u"qrc:/qt/qml/com/vicr123/thebeat/plugin/gst/GstCdPane.qml"_s), parent}, d{new GstCdPluginMediaSourcePrivate()} {
    d->parent = parent;
    connect(d->parent, &GstCdController::albumNameChanged, this, [this] {
        this->setName(d->parent->albumName());
    });
    this->setName(d->parent->albumName());
}

GstCdPluginMediaSource::~GstCdPluginMediaSource() {
    delete d;
}

GstCdController* GstCdPluginMediaSource::controller() {
    return d->parent;
}
