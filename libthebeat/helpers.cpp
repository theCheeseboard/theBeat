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
#include "helpers.h"

#include <QFile>
#include <QImage>
#include <QUrl>

#include <taglib/attachedpictureframe.h>
#include <taglib/fileref.h>
#include <taglib/flacfile.h>
#include <taglib/id3v2frame.h>
#include <taglib/id3v2framefactory.h>
#include <taglib/id3v2header.h>
#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>
#include <taglib/synchronizedlyricsframe.h>
#include <taglib/tbytevectorstream.h>
#include <taglib/xiphcomment.h>

#include <QDir>
#include <QFileInfo>

QCache<QUrl, QImage> Helpers::artCache(50000);
QMap<QMediaMetaData::Key, QString> Helpers::metadataStrings = {
    {QMediaMetaData::Title, "Title"},
    {QMediaMetaData::Author, "Author"},
    {QMediaMetaData::Comment, "Comment"},
    {QMediaMetaData::Description, "Description"},
    {QMediaMetaData::Genre, "Genre"},
    {QMediaMetaData::Date, "Date"},
    {QMediaMetaData::Language, "Language"},
    {QMediaMetaData::Publisher, "Publisher"},
    {QMediaMetaData::Copyright, "Copyright"},
    {QMediaMetaData::Url, "Url"},
    {QMediaMetaData::Duration, "Duration"},
    {QMediaMetaData::MediaType, "MediaType"},
    {QMediaMetaData::FileFormat, "FileFormat"},
    {QMediaMetaData::AudioBitRate, "AudioBitRate"},
    {QMediaMetaData::AudioCodec, "AudioCodec"},
    {QMediaMetaData::VideoBitRate, "VideoBitRate"},
    {QMediaMetaData::VideoCodec, "VideoCodec"},
    {QMediaMetaData::VideoFrameRate, "VideoFrameRate"},
    {QMediaMetaData::AlbumTitle, "AlbumTitle"},
    {QMediaMetaData::AlbumArtist, "AlbumArtist"},
    {QMediaMetaData::ContributingArtist, "ContributingArtist"},
    {QMediaMetaData::TrackNumber, "TrackNumber"},
    {QMediaMetaData::Composer, "Composer"},
    {QMediaMetaData::ThumbnailImage, "ThumbnailImage"},
    {QMediaMetaData::CoverArtImage, "CoverArtImage"},
    {QMediaMetaData::Orientation, "Orientation"},
    {QMediaMetaData::Resolution, "Resolution"}
};

tPromise<QImage>* Helpers::albumArt(QUrl url) {
    return tPromise<QImage>::runOnSameThread([=](tPromiseFunctions<QImage>::SuccessFunction res, tPromiseFunctions<QImage>::FailureFunction rej) {
        return tPromise<QImage>::runOnNewThread([=](tPromiseFunctions<QImage>::SuccessFunction res, tPromiseFunctions<QImage>::FailureFunction rej) {
            Q_UNUSED(rej)

            if (artCache.contains(url)) {
                res(artCache.object(url)->copy());
                return;
            }

            auto extractID3v2 = [](TagLib::ID3v2::Tag* tag) {
                TagLib::ID3v2::FrameList frameList = tag->frameListMap()["APIC"];
                if (frameList.isEmpty()) return QImage();

                TagLib::ID3v2::AttachedPictureFrame* frame;
                for (TagLib::ID3v2::FrameList::ConstIterator i = frameList.begin(); i != frameList.end(); i++) {
                    frame = static_cast<TagLib::ID3v2::AttachedPictureFrame*>(*i);
                    if (frame->type() == TagLib::ID3v2::AttachedPictureFrame::FrontCover) {
                        return QImage::fromData(reinterpret_cast<const uchar*>(frame->picture().data()), frame->picture().size());
                    }
                }

                return QImage();
            };

            auto extractXiph = [](TagLib::Ogg::XiphComment* xiph) {
                for (TagLib::FLAC::Picture* picture : xiph->pictureList()) {
                    if (picture->type() == TagLib::FLAC::Picture::FrontCover) {
                        return QImage::fromData(reinterpret_cast<const uchar*>(picture->data().data()), picture->data().size());
                    }
                }
                return QImage();
            };

            // Try using Taglib
            if (url.isLocalFile()) {
#ifndef Q_OS_WIN
#ifdef Q_OS_WIN
                TagLib::FileName filename = reinterpret_cast<const wchar_t*>(url.toLocalFile().constData());
#else
                TagLib::FileName filename = url.toLocalFile().toUtf8();
#endif

                TagLib::MPEG::File mpegFile(filename);
                if (mpegFile.hasID3v2Tag()) {
                    QImage image = extractID3v2(mpegFile.ID3v2Tag());
                    if (!image.isNull()) {
                        res(image);
                        return;
                    }
                }

                TagLib::FLAC::File flacFile(filename);
                if (flacFile.hasID3v2Tag()) {
                    QImage image = extractID3v2(flacFile.ID3v2Tag());
                    if (!image.isNull()) {
                        res(image);
                        return;
                    }
                }
                if (flacFile.hasXiphComment()) {
                    QImage image = extractXiph(flacFile.xiphComment());
                    if (!image.isNull()) {
                        res(image);
                        return;
                    }
                }
#endif
                // See if there is a cover file in the same directory
                QDir parentDir = QFileInfo(url.toLocalFile()).dir();
                for (const QFileInfo& coverPath : parentDir.entryInfoList({"cover.*"})) {
                    QImage image;
                    image.load(coverPath.absoluteFilePath());
                    if (!image.isNull()) {
                        res(image);
                        return;
                    }
                }
            } else if (url.scheme() == "qrc") {
                QFile qrc(url.path());
                qrc.open(QFile::ReadOnly);

                QByteArray ba = qrc.readAll();

                TagLib::ByteVectorStream* stream = new TagLib::ByteVectorStream(TagLib::ByteVector(ba.data(), ba.length()));
                TagLib::MPEG::File mpegFile(stream, TagLib::ID3v2::FrameFactory::instance());

                QImage image;
                if (mpegFile.hasID3v2Tag()) image = extractID3v2(mpegFile.ID3v2Tag());
                delete stream;
                if (!image.isNull()) {
                    res(image);
                    return;
                }
            }

            res(QImage());
        })->then([=](QImage art) {
            artCache.insert(url, new QImage(art), art.sizeInBytes());
            res(art);
        });
    });
}
QString Helpers::stringForMetadataKey(QMediaMetaData::Key key) {
  return metadataStrings.value(key);
}
QMediaMetaData::Key Helpers::metadataKeyForString(QString string) {
  return metadataStrings.key(string);
}
