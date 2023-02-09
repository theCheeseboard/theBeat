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
#include "basiclrcfilelyricformat.h"

#include <QMap>
#include <QRegularExpression>

struct BasicLrcFileLyricFormatPrivate {
        QMap<quint64, QString> lyrics;
};

BasicLrcFileLyricFormat::BasicLrcFileLyricFormat() :
    AbstractLyricFormat() {
    d = new BasicLrcFileLyricFormatPrivate();
}

BasicLrcFileLyricFormat::~BasicLrcFileLyricFormat() {
    delete d;
}

BasicLrcFileLyricFormat* BasicLrcFileLyricFormat::create(QString contents) {
    BasicLrcFileLyricFormat* format = new BasicLrcFileLyricFormat();
    QRegularExpression lineExpression(QRegularExpression::anchoredPattern("\\[(.+)\\](.+)?"));
    QRegularExpression timecodeExpression("(.+):(.+)\\.(.+)");
    for (const QString& line : contents.split("\n")) {
        QRegularExpressionMatch match = lineExpression.match(line);
        if (!match.hasMatch()) continue; // Not interested in this line
        QString timecode = match.captured(1);
        QString lyrics = match.captured(2);

        if (!timecode.at(0).isDigit()) continue; // This is a tag
        QRegularExpressionMatch timecodeMatch = timecodeExpression.match(timecode);
        QString minutes = timecodeMatch.captured(1).rightJustified(3, '0', true);
        QString seconds = timecodeMatch.captured(2).leftJustified(2, '0', true);
        QString millis = timecodeMatch.captured(3).leftJustified(3, '0', true);

        bool ok;
        quint64 timePoint = 0;
        timePoint += minutes.toInt(&ok) * 60 * 1000;
        if (!ok) continue;
        timePoint += seconds.toInt(&ok) * 1000;
        if (!ok) continue;
        timePoint += millis.toInt(&ok);
        if (!ok) continue;

        auto trimmed = lyrics.trimmed();
        if (!trimmed.isEmpty() || !format->d->lyrics.contains(timePoint)) {
            format->d->lyrics.insert(timePoint, lyrics.trimmed());
        }
    }
    return format;
}

QString BasicLrcFileLyricFormat::lyricsForTime(quint64 time, int offset) {
    QList<quint64> timePoints = d->lyrics.keys();
    for (auto timePoint = timePoints.crbegin(); timePoint != timePoints.crend(); timePoint++) {
        if (*timePoint <= time) {
            // Correct lyric for offset = 0
            while (offset != 0) {
                if (offset < 0) {
                    timePoint++;
                    if (timePoint == timePoints.crend()) return "";
                    offset++;
                } else if (offset > 0) {
                    if (timePoint == timePoints.crbegin()) return "";
                    timePoint--;
                    offset--;
                }
            }
            return d->lyrics.value(*timePoint);
        }
    }
    return "";
}

quint64 BasicLrcFileLyricFormat::timePointForTime(quint64 time, int offset) {
    QList<quint64> timePoints = d->lyrics.keys();
    for (auto timePoint = timePoints.crbegin(); timePoint != timePoints.crend(); timePoint++) {
        if (*timePoint <= time) {
            // Correct lyric for offset = 0
            while (offset != 0) {
                if (offset < 0) {
                    timePoint++;
                    if (timePoint == timePoints.crend()) return 0;
                    offset++;
                } else if (offset > 0) {
                    if (timePoint == timePoints.crbegin()) return 0;
                    timePoint--;
                    offset--;
                }
            }
            return *timePoint;
        }
    }
    return 0;
}
