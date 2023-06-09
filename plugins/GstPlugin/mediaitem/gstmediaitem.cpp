#include "gstmediaitem.h"

#include <QImage>
#include <QTimer>
#include <playlist.h>
#include <statemanager.h>
#include <tlogger.h>

struct GstMediaItemPrivate {
        QTimer* timer;
        bool prepared = false;

        QString title;
        QString album;
        QString artist;
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
                    case GST_MESSAGE_TAG:
                        {
                            GstTagList* tags = nullptr;
                            gst_message_parse_tag(msg, &tags);
                            gst_tag_list_foreach(
                                tags, [](const GstTagList* list, const gchar* tag, gpointer data) {
                                    auto self = reinterpret_cast<GstMediaItem*>(data);

                                    auto tagName = QString::fromLatin1(tag);
                                    guint tagSize = gst_tag_list_get_tag_size(list, tag);
                                    for (auto i = 0; i < tagSize; i++) {
                                        auto tagValue = gst_tag_list_get_value_index(list, tag, i);
                                        if (G_VALUE_HOLDS_STRING(tagValue)) {
                                            auto str = QString::fromLocal8Bit(g_value_get_string(tagValue));
                                            tWarn("GstMediaItem") << "Tag received: " << tagName << " -> " << str;

                                            if (tagName == GST_TAG_TITLE) {
                                                self->d->title = str;
                                            } else if (tagName == GST_TAG_ALBUM) {
                                                self->d->album = str;
                                            } else if (tagName == GST_TAG_ALBUM_ARTIST) {
                                                self->d->artist = str;
                                            } else if (tagName == "musicbrainz-discid") {
                                                // TODO: MusicBrainz support
                                            }
                                        }
                                    }

                                    emit self->metadataChanged();
                                },
                                data);
                            gst_tag_list_unref(tags);
                        }
                    default:
                        break;
                }

                return true;
            },
            this);
        gst_object_unref(bus);

        connect(StateManager::instance()->playlist(), &Playlist::logAdjustedVolumeChanged, this, [this](double volume) {
            auto volumeElement = gst_bin_get_by_name(GST_BIN(pipeline()), "volume");
            g_object_set(volumeElement, "volume", volume, nullptr);
            gst_object_unref(volumeElement);
        });
        auto volumeElement = gst_bin_get_by_name(GST_BIN(pipeline()), "volume");
        g_object_set(volumeElement, "volume", StateManager::instance()->playlist()->logAdjustedVolume(), nullptr);
        gst_object_unref(volumeElement);

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
    return d->title;
}

QStringList GstMediaItem::authors() {
    return {d->artist};
}

QString GstMediaItem::album() {
    return d->album;
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

QVariant GstMediaItem::metadata(QString key) {
    return QVariant();
}
