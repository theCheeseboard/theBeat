/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2022 Victor Tran
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
#ifndef ABSTRACTLYRICFORMAT_H
#define ABSTRACTLYRICFORMAT_H

#include <QObject>

class AbstractLyricFormat : public QObject
{
        Q_OBJECT
    public:
        explicit AbstractLyricFormat();

        static AbstractLyricFormat* loadLyricFile(QString format, QString contents);

        virtual QString lyricsForTime(quint64 time, int offset = 0) = 0;
        virtual quint64 timePointForTime(quint64 time, int offset = 0) = 0;

    signals:

};

#endif // ABSTRACTLYRICFORMAT_H
