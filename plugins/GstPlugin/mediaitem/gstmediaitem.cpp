#include "gstmediaitem.h"

#include <QImage>
#include <QTimer>
#include <tlogger.h>

struct GstMediaItemPrivate {
        QTimer* timer;
        bool prepared = false;
};

GstMediaItem::GstMediaItem() {
    d = new GstMediaItemPrivate();

    d->timer = new QTimer(this);
    d->timer->setInterval(100);
    connect(d->timer, &QTimer::timeout, this, &GstMediaItem::elapsedChanged);
    //    connect(d->timer, &QTimer::timeout, this, &GstMediaItem::durationChanged);
    d->timer->start();
}

GstMediaItem::~GstMediaItem() {
    d->timer->deleteLater();
    delete d;
}

void GstMediaItem::preparePlayer() {
    if (!d->prepared) {
        auto bus = gst_element_get_bus(pipeline());
        gst_bus_add_watch(
            bus, [](GstBus* bus, GstMessage* msg, gpointer data) -> gboolean {
                auto self = reinterpret_cast<GstMediaItem*>(data);

                switch (msg->type) {
                    case GST_MESSAGE_EOS:
                        emit self->done();
                        break;
                    case GST_MESSAGE_DURATION_CHANGED:
                        emit self->durationChanged();
                        break;
                    case GST_MESSAGE_ERROR:
                        tWarn("GstMediaItem") << "Gst Media Item signalled an error";
                        emit self->error();
                        break;
                    default:
                        break;
                }

                return true;
            },
            this);
        gst_object_unref(bus);
        d->prepared = true;
    }
}

void GstMediaItem::play() {
    preparePlayer();
    gst_element_set_state(pipeline(), GST_STATE_PLAYING);
}

void GstMediaItem::pause() {
    preparePlayer();
    gst_element_set_state(pipeline(), GST_STATE_PAUSED);
}

void GstMediaItem::stop() {
    preparePlayer();
    gst_element_set_state(pipeline(), GST_STATE_READY);
}

void GstMediaItem::seek(quint64 ms) {
    preparePlayer();
    auto seconds = (GST_SECOND * ms) / 1000.0;
    gst_element_seek(pipeline(), 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, seconds, GST_SEEK_TYPE_NONE, -1);
}

quint64 GstMediaItem::elapsed() {
    preparePlayer();
    gint64 seconds;
    gst_element_query_position(pipeline(), GST_FORMAT_TIME, &seconds);
    return seconds * 1000.0 / GST_SECOND;
}

quint64 GstMediaItem::duration() {
    preparePlayer();
    gint64 seconds;
    gst_element_query_duration(pipeline(), GST_FORMAT_TIME, &seconds);
    return seconds * 1000.0 / GST_SECOND;
}

QString GstMediaItem::title() {
    return "";
}

QStringList GstMediaItem::authors() {
    return {};
}

QString GstMediaItem::album() {
    return "";
}

QImage GstMediaItem::albumArt() {
    return QImage();
}

QString GstMediaItem::lyrics() {
    return "";
}

QString GstMediaItem::lyricFormat() {
    return "";
}
