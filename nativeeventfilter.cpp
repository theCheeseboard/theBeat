#include "nativeeventfilter.h"

NativeEventFilter::NativeEventFilter(MainWindow* parent)
{
    lastPress.start();
    this->parent = parent;
}

bool NativeEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result) {
    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t* event = static_cast<xcb_generic_event_t*>(message);
        if (event->response_type == XCB_KEY_PRESS) {
            if (lastPress.restart() > 100) {
                xcb_key_press_event_t* button = static_cast<xcb_key_press_event_t*>(message);

                if (button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_AudioPlay) || button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_AudioStop)) {
                    if (parent->getPlayer()->state() == Phonon::PlayingState) {
                        parent->getPlayer()->pause();
                    } else {
                        parent->getPlayer()->play();
                    }
                } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_AudioNext)) {
                    parent->getPlaylist()->playNext();
                } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_AudioPrev)) {
                    parent->getPlaylist()->skipBack();
                }
            }
        }
    }
    return false;
}
