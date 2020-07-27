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
#ifndef MPRISINSTANCE_H
#define MPRISINSTANCE_H

#include <QDBusAbstractAdaptor>

class MprisInstance : public QDBusAbstractAdaptor {
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2")

        Q_PROPERTY(bool CanQuit READ CanQuit)
        Q_PROPERTY(bool Fullscreen READ Fullscreen WRITE setFullscreen)
        Q_PROPERTY(bool CanSetFullscreen READ CanSetFullscreen)
        Q_PROPERTY(bool CanRaise READ CanRaise)
        Q_PROPERTY(bool HasTrackList READ HasTrackList)
        Q_PROPERTY(QString Identity READ Identity)
        Q_PROPERTY(QString DesktopEntry READ DesktopEntry)
        Q_PROPERTY(QStringList SupportedUriSchemes READ SupportedUriSchemes)
        Q_PROPERTY(QStringList SupportedMimeTypes READ SupportedMimeTypes)

    public:
        explicit MprisInstance(QObject* parent = nullptr);

        bool CanQuit();

        bool Fullscreen();
        void setFullscreen(bool fullscreen);

        bool CanSetFullscreen();

        bool CanRaise();

        bool HasTrackList();

        QString Identity();

        QString DesktopEntry();

        QStringList SupportedUriSchemes();

        QStringList SupportedMimeTypes();

    public slots:
        Q_SCRIPTABLE void Raise();
        Q_SCRIPTABLE void Quit();

    signals:

};

#endif // MPRISPLAYER_H
