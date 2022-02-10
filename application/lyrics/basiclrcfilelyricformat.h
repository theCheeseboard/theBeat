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
#ifndef BASICLRCFILELYRICFORMAT_H
#define BASICLRCFILELYRICFORMAT_H

#include "abstractlyricformat.h"

struct BasicLrcFileLyricFormatPrivate;
class BasicLrcFileLyricFormat : public AbstractLyricFormat
{
        Q_OBJECT
    public:
        ~BasicLrcFileLyricFormat();

        static BasicLrcFileLyricFormat* create(QString contents);

        QString lyricsForTime(quint64 time, int offset = 0);
        quint64 timePointForTime(quint64 time, int offset = 0);

    signals:

    private:
        explicit BasicLrcFileLyricFormat();
        BasicLrcFileLyricFormatPrivate* d;
};

#endif // BASICLRCFILELYRICFORMAT_H
