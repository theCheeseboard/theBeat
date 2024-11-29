#include "wincdpluginmediasource.h"

#include "cdchecker.h"

struct WinCdPluginMediaSourcePrivate {
    CdChecker* parent;
};

WinCdPluginMediaSource::WinCdPluginMediaSource(CdChecker* parent) :
    PluginMediaSource{nullptr, QUrl(u"qrc:/qt/qml/com/vicr123/thebeat/plugin/winintegration/WinCdPane.qml"_qs), parent}, d{new WinCdPluginMediaSourcePrivate()} {
    d->parent = parent;

    connect(parent, &CdChecker::albumNameChanged, this, [parent, this] {
        this->setName(parent->albumName());
    });
}

WinCdPluginMediaSource::~WinCdPluginMediaSource() {
    delete d;
}

CdChecker* WinCdPluginMediaSource::controller() {
    return d->parent;
}
