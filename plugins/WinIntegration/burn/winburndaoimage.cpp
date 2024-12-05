#include "winburndaoimage.h"

#include <comutil.h>
#include <tlogger.h>
#include <Shlwapi.h>
#include <QAudioFormat>
#include <QAudioDecoder>
#include <QtConcurrent>
#include <QCoroFuture>
#include <QCoroSignal>
#include "cdtextgenerator.h"

#include <taglib/fileref.h>

struct WinBurnDaoImagePrivate {
    QList<qint64> trackOffsets;
    qint64 leadout;

    QString albumName;
    QStringList files;
    winrt::com_ptr<IRawCDImageCreator> daoImage;
};

WinBurnDaoImage::WinBurnDaoImage(QObject* parent) : QObject(parent) {
    d = new WinBurnDaoImagePrivate();
}

WinBurnDaoImage::~WinBurnDaoImage() {
    delete d;
}

QCoro::Task<QByteArray> transcode(const QString& file) {
    tDebug("WinBurnDaoImage") << "Start transcode " << file;
    QAudioFormat format;
    format.setSampleFormat(QAudioFormat::Int16);
    format.setSampleRate(44100);
    format.setChannelCount(2);

    QAudioDecoder decoder;

    decoder.setAudioFormat(format);
    decoder.setSource(QUrl(file));

    QByteArray audioData;

    QObject::connect(&decoder, &QAudioDecoder::bufferReady, [ &decoder, &audioData, file ] {
        QAudioBuffer buf = decoder.read();
        audioData.append(QByteArray(buf.data<char>(), buf.byteCount()));
    });
    auto finishConnection = qCoro(&decoder, &QAudioDecoder::finished);
    decoder.start();
    if (decoder.error() == QAudioDecoder::NoError) {
        co_await finishConnection;
    }

    if (decoder.error() != QAudioDecoder::NoError) {
        tWarn("WinBurnDaoImage") << "Transcode error " << file;
        tWarn("WinBurnDaoImage") << decoder.errorString();
        co_return {};
    }

    tDebug("WinBurnDaoImage") << "Finish transcode " << file;

    int extraBytes = audioData.size() % 2352;
    int paddingBytes = 2352 - extraBytes;
    audioData.append(QByteArray(paddingBytes, 0));

    co_return audioData;
}

QCoro::Task<> WinBurnDaoImage::createImageFromFiles(QStringList files) {
    QPointer<WinBurnDaoImage> thisPtr(this);

    auto daoImage = winrt::create_instance<IRawCDImageCreator>(CLSID_MsftRawCDImageCreator);
    QList<qint64> trackOffsets;

    std::vector<QCoro::Task<QByteArray>> transcodeTasks;
    for (const auto& file : files) {
        transcodeTasks.push_back(std::move(transcode(file)));
    }

    for (const auto& transcodingCoro : transcodeTasks) {
        auto transcoding = co_await transcodingCoro;

        tDebug("WinBurnDaoImage") << "Transcode track complete";

        winrt::com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(reinterpret_cast<const uchar*>(transcoding.constData()), transcoding.length()));

        LONG resultantTrack;
        winrt::check_hresult(daoImage->AddTrack(IMAPI_CD_SECTOR_AUDIO, stream.get(), &resultantTrack));

        winrt::com_ptr<IRawCDImageTrackInfo> trackInfo;
        winrt::check_hresult(daoImage->get_TrackInfo(resultantTrack, trackInfo.put()));

        LONG startingLba;
        winrt::check_hresult(trackInfo->get_StartingLba(&startingLba));

        trackOffsets.append(startingLba);
    }

    LONG startOfLeadout;
    winrt::check_hresult(daoImage->get_StartOfLeadout(&startOfLeadout));

    //TODO: Fix race condition
    if (!thisPtr) co_return;

    d->daoImage = daoImage;
    d->trackOffsets = trackOffsets;
    d->leadout = startOfLeadout;
    d->files = files;
}

winrt::com_ptr<IRawCDImageCreator> WinBurnDaoImage::daoImage() {
    return d->daoImage;
}

int WinBurnDaoImage::trackNumberFromLba(qint64 lba) {
    if (lba >= d->leadout) {
        return -2;
    } else {
        int track = -1;
        for (int i = d->trackOffsets.count() - 1; i >= 0; i--) {
            if (lba >= d->trackOffsets.at(i)) {
                track = i;
                break;
            }
        }

        if (track == -1) {
            return -1;
        } else {
            return track + 1;
        }
    }

}

qint64 WinBurnDaoImage::leadoutLba() {
    return d->leadout;
}

void WinBurnDaoImage::setAlbumName(QString albumName) {
    d->albumName = albumName;

    CDTextGeneratorPtr cdtgen(new CDTextGenerator());
    cdtgen->addTrack({ //Disc Information
        d->albumName,
        "", "", "", ""
    });

    for (int i = 0; i < d->files.count(); i++) {
        TagLib::FileRef file(d->files.at(i).toStdString().data());
        if (file.tag()) {
            cdtgen->addTrack({
                file.tag()->title().toCString(),
                file.tag()->artist().toCString(),
                file.tag()->artist().toCString(),
                file.tag()->artist().toCString(),
                file.tag()->artist().toCString()
            });
        } else {
            cdtgen->addTrack({"", "", "", "", ""});
        }
    }

    LONG lastSector;
    winrt::check_hresult(d->daoImage->get_LastUsedUserSectorInImage(&lastSector));

    QByteArray cdTextBytes = cdtgen->generate();
    cdTextBytes.resize(lastSector * 96);

    winrt::com_ptr<IStream> stream;
    stream.attach(SHCreateMemStream(reinterpret_cast<const uchar*>(cdTextBytes.constData()), cdTextBytes.count()));
    winrt::check_hresult(d->daoImage->AddSubcodeRWGenerator(stream.get()));
}
