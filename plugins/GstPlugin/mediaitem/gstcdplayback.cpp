#include "gstcdplayback.h"

struct GstCdPlaybackPrivate {
        GstElement* pipeline = nullptr;

        QString device;
        int track;
        GstTrackInfoPtr trackInfo;
};

GstCdPlayback::GstCdPlayback(QString device, int track, GstTrackInfoPtr trackInfo) :
    GstMediaItem() {
    d = new GstCdPlaybackPrivate();
    d->track = track;
    d->device = device;
    d->trackInfo = trackInfo;
}

GstCdPlayback::~GstCdPlayback() {
    if (d->pipeline) {
        gst_element_set_state(d->pipeline, GST_STATE_NULL);
        gst_object_unref(d->pipeline);
    }
    delete d;
}

GstElement* GstCdPlayback::pipeline() {
    return d->pipeline;
}

QString GstCdPlayback::title() {
    return d->trackInfo->title();
}

void GstCdPlayback::preparePlayer() {
    if (!d->pipeline) {
        auto pipelineString = QStringLiteral("cdiocddasrc device=%1 track=%2 ! volume name=\"volume\" ! pulsesink").arg(d->device).arg(d->track);
        d->pipeline = gst_parse_launch(pipelineString.toUtf8().data(), nullptr);
    }
    GstMediaItem::preparePlayer();
}

QVariant GstCdPlayback::metadata(QString key) {
    auto metadata = GstMediaItem::metadata(key);
    if (!metadata.isNull()) return metadata;

    if (key == "TrackNumber") {
        return d->track;
    }

    return QVariant();
}

QStringList GstCdPlayback::authors() {
    return d->trackInfo->artist();
}

QString GstCdPlayback::album() {
    return d->trackInfo->album();
}
