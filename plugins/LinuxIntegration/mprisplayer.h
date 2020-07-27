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
#ifndef MPRISPLAYER_H
#define MPRISPLAYER_H

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

class MediaItem;
struct MprisPlayerPrivate;
class MprisPlayer : public QDBusAbstractAdaptor {
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")

        Q_PROPERTY(QString PlaybackStatus READ PlaybackStatus)
        Q_PROPERTY(QString LoopStatus READ LoopStatus WRITE setLoopStatus)
        Q_PROPERTY(double Rate READ Rate WRITE setRate)
        Q_PROPERTY(bool Shuffle READ Shuffle WRITE setShuffle)
        Q_PROPERTY(QVariantMap Metadata READ Metadata)
        Q_PROPERTY(double Volume READ Volume WRITE setVolume)
        Q_PROPERTY(qint64 Position READ Position)
        Q_PROPERTY(double MinimumRate READ MinimumRate)
        Q_PROPERTY(double MaximumRate READ MaximumRate)
        Q_PROPERTY(bool CanGoNext READ CanGoNext)
        Q_PROPERTY(bool CanGoPrevious READ CanGoPrevious)
        Q_PROPERTY(bool CanPlay READ CanPlay)
        Q_PROPERTY(bool CanPause READ CanPause)
        Q_PROPERTY(bool CanSeek READ CanSeek)
        Q_PROPERTY(bool CanControl READ CanControl)

    public:
        explicit MprisPlayer(QObject* parent = nullptr);
        ~MprisPlayer();

        QString PlaybackStatus();

        QString LoopStatus();
        void setLoopStatus(QString loopStatus);

        double Rate();
        void setRate(double rate);

        bool Shuffle();
        void setShuffle(bool shuffle);

        QVariantMap Metadata();

        double Volume();
        void setVolume(double volume);

        qint64 Position();

        double MinimumRate();

        double MaximumRate();

        bool CanGoNext();

        bool CanGoPrevious();

        bool CanPlay();

        bool CanPause();

        bool CanSeek();

        bool CanControl();

    public slots:
        Q_SCRIPTABLE void Next();
        Q_SCRIPTABLE void Previous();
        Q_SCRIPTABLE void Pause();
        Q_SCRIPTABLE void PlayPause();
        Q_SCRIPTABLE void Stop();
        Q_SCRIPTABLE void Play();
        Q_SCRIPTABLE void Seek(qint64 us);
        Q_SCRIPTABLE void SetPosition(QDBusObjectPath trackId, qint64 us);
        Q_SCRIPTABLE void OpenUri(QString uri);

    signals:
        Q_SCRIPTABLE void Seeked(qint64 us);

    private:
        MprisPlayerPrivate* d;

        void updateCurrentItem();
        void propertyChanged(QString property);
        QDBusObjectPath trackPath(MediaItem* item);

};

#endif // MPRISPLAYER_H
