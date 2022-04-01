/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "nativeevents.h"

#include <QApplication>
#include <statemanager.h>
#include <playlist.h>

#ifdef HAVE_X11
    #include <tx11info.h>

    #include <X11/Xlib.h>
    #include <X11/XF86keysym.h>
    #include <xcb/xcb.h>
    #include <xcb/xproto.h>
#endif

NativeEvents::NativeEvents(QObject* parent) : QObject(parent) {
    QApplication::instance()->installNativeEventFilter(this);

#ifdef HAVE_X11
    if (tX11Info::isPlatformX11()) {
        //Capture keys using X11
        XGrabKey(tX11Info::display(), XKeysymToKeycode(tX11Info::display(), XF86XK_AudioPlay), AnyModifier, RootWindow(tX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
        XGrabKey(tX11Info::display(), XKeysymToKeycode(tX11Info::display(), XF86XK_AudioNext), AnyModifier, RootWindow(tX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
        XGrabKey(tX11Info::display(), XKeysymToKeycode(tX11Info::display(), XF86XK_AudioPrev), AnyModifier, RootWindow(tX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
        XGrabKey(tX11Info::display(), XKeysymToKeycode(tX11Info::display(), XF86XK_AudioStop), AnyModifier, RootWindow(tX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    }
#endif
}

bool NativeEvents::nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) {
#ifdef HAVE_X11
    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t* event = static_cast<xcb_generic_event_t*>(message);
        if (event->response_type == XCB_KEY_PRESS) {
            xcb_key_press_event_t* button = static_cast<xcb_key_press_event_t*>(message);

            if (button->detail == XKeysymToKeycode(tX11Info::display(), XF86XK_AudioPlay) || button->detail == XKeysymToKeycode(tX11Info::display(), XF86XK_AudioStop)) {
                if (StateManager::instance()->playlist()->state() == Playlist::Playing) {
                    StateManager::instance()->playlist()->pause();
                } else {
                    StateManager::instance()->playlist()->play();
                }
            } else if (button->detail == XKeysymToKeycode(tX11Info::display(), XF86XK_AudioNext)) {
                StateManager::instance()->playlist()->next();
            } else if (button->detail == XKeysymToKeycode(tX11Info::display(), XF86XK_AudioPrev)) {
                StateManager::instance()->playlist()->previous();
            }
        }
    }
#endif

    return false;
}
