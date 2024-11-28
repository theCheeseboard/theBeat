#include "musicbrainzclient.h"

#include <QCoroFuture>
#include <QCryptographicHash>
#include <QImage>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QtConcurrent>

#ifdef HAVE_MUSICBRAINZ
    #include <musicbrainz5/Artist.h>
    #include <musicbrainz5/ArtistCredit.h>
    #include <musicbrainz5/Medium.h>
    #include <musicbrainz5/NameCredit.h>
    #include <musicbrainz5/Query.h>
    #include <musicbrainz5/Recording.h>
    #include <musicbrainz5/Release.h>
    #include <musicbrainz5/Track.h>
#endif

using namespace Qt::Literals;

struct MusicBrainzClientPrivate {
        QNetworkAccessManager mgr;
        QString discId;
        QImage albumArt;
        QString albumName;
        QList<MusicBrainzClient::MusicBrainzTrack> tracks;

#ifdef HAVE_MUSICBRAINZ
        QString currentReleaseId;
        MusicBrainz5::CReleaseList releases;
#endif
};

MusicBrainzClient::MusicBrainzClient(int firstTrack, int lastTrack, int leadOut, int frameOffsets[99], QObject* parent) :
    QObject{parent}, d{new MusicBrainzClientPrivate} {
    QString data;
    data.append(QString::asprintf("%02X", firstTrack));
    data.append(QString::asprintf("%02X", lastTrack));
    data.append(QString::asprintf("%08X", leadOut));

    for (int i = 0; i < 99; i++) {
        data.append(QString::asprintf("%08X", frameOffsets[i]));
    }

    auto hash = QCryptographicHash::hash(data.toLatin1(), QCryptographicHash::Sha1);
    d->discId = hash.toBase64(QByteArray::Base64Encoding).replace("+", ".").replace("/", "_").replace("=", "-");

    this->loadMusicbrainzData();
}

MusicBrainzClient::~MusicBrainzClient() {
    delete d;
}

QString MusicBrainzClient::albumName() {
    return d->albumName;
}

QImage MusicBrainzClient::albumArt() {
    return d->albumArt;
}

MusicBrainzClient::MusicBrainzTrack MusicBrainzClient::track(int index) {
    return d->tracks.at(index);
}

int MusicBrainzClient::trackCount() {
    return d->tracks.length();
}

QCoro::Task<> MusicBrainzClient::loadMusicbrainzData() {
    // Load information from MusicBrainz
#ifdef HAVE_MUSICBRAINZ
    QPointer<QObject> context = this;

    auto releases = co_await QtConcurrent::run([](QString discId) {
        MusicBrainz5::CQuery query("thebeat-3.0");
        try {
            return query.LookupDiscID(discId.toStdString());
        } catch (...) {
            return MusicBrainz5::CReleaseList{};
        }
    }, d->discId);

    if (!context) co_return;
    d->releases = releases;

    if (d->releases.Count() > 0) {
        selectMusicbrainzRelease(QString::fromStdString(d->releases.Item(0)->ID()));
    }
#else
    co_return;
#endif
}

QCoro::Task<> MusicBrainzClient::selectMusicbrainzRelease(QString release) {
#ifdef HAVE_MUSICBRAINZ
    d->currentReleaseId = release;
    d->albumArt = QImage();
    emit albumArtChanged();

    auto releaseInfo = co_await QtConcurrent::run([](QString release) -> MusicBrainz5::CRelease* {
        MusicBrainz5::CQuery query("thebeat-3.0");
        MusicBrainz5::CQuery::tParamMap params;
        params["inc"] = "artists labels recordings release-groups url-rels discids artist-credits";
        MusicBrainz5::CMetadata fullData = query.Query("release", release.toStdString(), "", params);
        if (fullData.Release()) {
            return new MusicBrainz5::CRelease(*fullData.Release());
        } else {
            return nullptr;
        }
    }, release);

    // Make sure the user hasn't changed releases
    if (d->currentReleaseId != release) co_return;

    d->albumName = QString::fromStdString(releaseInfo->Title());
    emit albumNameChanged();

    // Attempt to get album art for this release
    auto releaseId = QString::fromStdString(releaseInfo->ID());
    QNetworkRequest req(QUrl(u"https://coverartarchive.org/release/%1/front"_s.arg(releaseId)));
    QNetworkReply* artReply = d->mgr.get(req);
    connect(artReply, &QNetworkReply::finished, this, [this, release, artReply] {
        // Make sure the user hasn't changed releases
        if (d->currentReleaseId != release) return;

        if (artReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
            d->albumArt = QImage::fromData(artReply->readAll());
            emit albumArtChanged();
        }
    });

    d->tracks.clear();
    MusicBrainz5::CMediumList* mediumList = releaseInfo->MediumList();
    for (int h = 0; h < mediumList->NumItems(); h++) {
        MusicBrainz5::CMedium* medium = mediumList->Item(h);
        if (medium->ContainsDiscID(d->discId.toStdString())) {
            MusicBrainz5::CTrackList* tracks = medium->TrackList();
            for (int i = 0; i < tracks->Count(); i++) {
                MusicBrainz5::CTrack* track = tracks->Item(i);
                MusicBrainz5::CRecording* recording = track->Recording();
                if (!recording) continue;

                QStringList artists;
                MusicBrainz5::CNameCreditList* nameCreditList = recording->ArtistCredit()->NameCreditList();
                for (int j = 0; j < nameCreditList->NumItems(); j++) {
                    MusicBrainz5::CNameCredit* credit = nameCreditList->Item(j);
                    artists.append(QString::fromStdString(credit->Artist()->Name()));
                }
                artists.removeDuplicates();

                MusicBrainzTrack mbTrack{
                    QString::fromStdString(recording->Title()),
                    artists,
                    QString::fromStdString(releaseInfo->Title())};
                d->tracks.append(mbTrack);
            }

            break;
        }
    }

    emit tracksChanged();
    delete releaseInfo;
#else
    co_return;
#endif
}
