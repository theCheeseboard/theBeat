#include "maccdpluginmediasource.h"

struct MacCdPluginMediaSourcePrivate {
        CdChecker* parent;
};

MacCdPluginMediaSource::MacCdPluginMediaSource(CdChecker* parent) :
    PluginMediaSource{nullptr, QUrl(u"qrc:/qt/qml/com/vicr123/thebeat/plugin/macintegration/MacCdPane.qml"_qs), parent}, d{new MacCdPluginMediaSourcePrivate()} {
    d->parent = parent;

    connect(parent, &CdChecker::albumNameChanged, this, [parent, this] {
        this->setName(parent->albumName());
    });
}

MacCdPluginMediaSource::~MacCdPluginMediaSource() {
    delete d;
}

CdChecker* MacCdPluginMediaSource::controller() {
    return d->parent;
}
