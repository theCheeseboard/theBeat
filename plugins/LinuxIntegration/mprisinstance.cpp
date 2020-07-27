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
#include "mprisinstance.h"

#include <QApplication>

MprisInstance::MprisInstance(QObject* parent) : QDBusAbstractAdaptor(parent) {

}

bool MprisInstance::CanQuit() {
    return true;
}

bool MprisInstance::Fullscreen() {
    return false;
}

void MprisInstance::setFullscreen(bool fullscreen) {
    //noop
    Q_UNUSED(fullscreen)
}

bool MprisInstance::CanSetFullscreen() {
    return false;
}

bool MprisInstance::CanRaise() {
    return false;
}

bool MprisInstance::HasTrackList() {
    return false;
}

QString MprisInstance::Identity() {
    return QStringLiteral("theBeat");
}

QString MprisInstance::DesktopEntry() {
    return QStringLiteral("com.vicr123.thebeat");
}

QStringList MprisInstance::SupportedUriSchemes() {
    return {
        QStringLiteral("file"),
        QStringLiteral("http"),
        QStringLiteral("https")
    };
}

QStringList MprisInstance::SupportedMimeTypes() {
    return {};
}

void MprisInstance::Raise() {

}

void MprisInstance::Quit() {
    QApplication::quit();
}
