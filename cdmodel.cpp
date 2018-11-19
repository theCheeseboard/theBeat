#include "cdmodel.h"

#include <QSize>
#include <QDBusInterface>
#include <phonon/MediaObject>
#include <phonon/MediaController>
#include <phonon/MediaSource>
#include <phonon/AudioDataOutput>
#include <tpromise.h>

#include <QNetworkRequest>
#include <QNetworkReply>

#include <musicbrainz5/Query.h>
#include <musicbrainz5/Disc.h>
#include <musicbrainz5/Release.h>
#include <musicbrainz5/ReleaseList.h>
#include <musicbrainz5/ReleaseGroup.h>
#include <musicbrainz5/Medium.h>
#include <musicbrainz5/TrackList.h>
#include <musicbrainz5/Track.h>
#include <musicbrainz5/Recording.h>
#include <musicbrainz5/ArtistCredit.h>
#include <musicbrainz5/NameCredit.h>
#include <musicbrainz5/Artist.h>

using namespace Phonon;

CdModel::CdModel(QString device, QObject *parent)
    : QAbstractTableModel(parent)
{
    this->device = device;

    //Get CD drive and attach to CD information
    QDBusInterface blockInterface("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2/block_devices/sr0", "org.freedesktop.UDisks2.Block", QDBusConnection::systemBus());
    cdDrivePath = blockInterface.property("Drive").value<QDBusObjectPath>();
    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", cdDrivePath.path(), "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(checkCd()));

    //Check CD once everything is initialized
    QTimer::singleShot(0, this, SLOT(checkCd()));
}

QVariant CdModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return tr("Name", "Name of a music track");
            case 1:
                return tr("Artist");
            case 2:
                return tr("Album");
        }
    } else if (role == Qt::SizeHintRole) {
        return QSize(500, 29);
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

int CdModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return trackData.count();
}

int CdModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return 3;
}

QVariant CdModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    int row = index.row();
    int col = index.column();

    switch (col) {
        case 0: { //Title
            switch (role) {
                case Qt::DisplayRole:
                    return trackData.at(index.row());
            }
            break;
        }
    }

    return QVariant();
}

void CdModel::checkCd() {
    //Check the CD asynchronously
    changeUiPane(1);

    struct CdInformation {
        bool available = false;
        int numberOfTracks = 0;
        QStringList mbDiscId;
    };

    (new tPromise<CdInformation*>([=](QString &error) -> CdInformation* {
        CdInformation* info = new CdInformation;
        if (cdDrivePath.path() == "") return info;

        QDBusInterface cdDriveInterface("org.freedesktop.UDisks2", cdDrivePath.path(), "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus());
        if (cdDriveInterface.property("MediaAvailable").toBool()) {

            QEventLoop* eventLoop = new QEventLoop();
            MediaObject* cdFinder = new MediaObject();
            MediaController* cdController = new MediaController(cdFinder);
            AudioDataOutput dummyOutput;
            createPath(cdFinder, &dummyOutput);
            connect(cdController, &MediaController::availableTitlesChanged, [=](int tracks) {
                //New CD inserted
                info->numberOfTracks = tracks;

                if (tracks > 0) {
                    connect(cdFinder, &MediaObject::metaDataChanged, [=]() {
                        info->mbDiscId = cdFinder->metaData(Phonon::MusicBrainzDiscIdMetaData);
                        eventLoop->quit();
                    });
                    cdController->setCurrentTitle(1);
                } else {
                    eventLoop->quit();
                }
            });

            cdFinder->setCurrentSource(MediaSource(Phonon::Cd, "/dev/sr0"));
            cdFinder->play();
            cdFinder->pause();

            eventLoop->exec();

            eventLoop->deleteLater();
            cdFinder->deleteLater();
            cdController->deleteLater();
            info->available = true;
        }
        return info;
    }))->then([=](CdInformation* info) {
        qDebug() << info->mbDiscId;
        trackData.clear();

        title = tr("CD Drive");
        art = QImage();
        if (info->numberOfTracks == 0) {
            changeUiPane(0);
        } else {
            for (int i = 0; i < info->numberOfTracks; i++) {
                trackData.append(tr("Track %n", nullptr, i + 1));
            }

            if (info->mbDiscId.count() != 0) {
                //Query MusicBrainz
                QStringList mbDiscId = info->mbDiscId;

                struct CddbInfo {
                    bool valid = false;
                    QString title;
                    QStringList tracks;
                    QImage art;
                };

                emit queryingCddb(true);
                (new tPromise<CddbInfo>([=](QString &error) -> CddbInfo {
                    MusicBrainz5::CQuery query("thebeat-2.0");
                    try {
                        MusicBrainz5::CMetadata data = query.Query("discid", mbDiscId.at(0).toStdString());
                        if (!data.Disc() || !data.Disc()->ReleaseList()) return CddbInfo();

                        MusicBrainz5::CReleaseList *releaseList = data.Disc()->ReleaseList();

                        for (int count = 0; count < releaseList->NumItems(); count++) {
                            MusicBrainz5::CRelease *release = releaseList->Item(count);
                            //The releases returned from LookupDiscID don't contain full information
                            MusicBrainz5::CQuery::tParamMap Params;
                            Params["inc"] = "artists labels recordings release-groups url-rels discids artist-credits";
                            MusicBrainz5::CMetadata fullData = query.Query("release", release->ID(), "", Params);
                            if (!fullData.Release()) continue;

                            MusicBrainz5::CRelease *FullRelease = fullData.Release();
                            MusicBrainz5::CMediumList MediaList = FullRelease->MediaMatchingDiscID(mbDiscId.at(0).toStdString());

                            if (MediaList.NumItems() == 0) continue;

                            for (int count = 0; count < MediaList.NumItems(); count++) {
                                MusicBrainz5::CMedium *Medium = MediaList.Item(count);

                                MusicBrainz5::CTrackList *TrackList = Medium->TrackList();
                                if (!TrackList) continue;

                                CddbInfo info;
                                info.title = QString::fromStdString(release->Title());

                                QNetworkAccessManager mgr;
                                QNetworkRequest req(QUrl("https://coverartarchive.org/release/" + QString::fromStdString(release->ID()) + "/front"));
                                req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
                                QNetworkReply* artReply = mgr.get(req);

                                for (int count = 0; count < TrackList->NumItems(); count++) {
                                    MusicBrainz5::CTrack *Track = TrackList->Item(count);
                                    MusicBrainz5::CRecording *Recording = Track->Recording();

                                    /*QMap<QString, QString> trackInfo;

                                    if (Recording) {
                                        trackInfo.insert("name", QString::fromStdString(Recording->Title()));
                                        trackInfo.insert("artist", QString::fromStdString(Recording->ArtistCredit()->NameCreditList()->Item(0)->Artist()->Name()));
                                    } else {
                                        trackInfo.insert("name", QString::fromStdString(Track->Title()));
                                        trackInfo.insert("artist", QString::fromStdString(Track->ArtistCredit()->NameCreditList()->Item(0)->Artist()->Name()));
                                    }

                                    trackInfo.insert("album", QString::fromStdString(FullRelease->ReleaseGroup()->Title()));
                                    //trackInfo.insert("artist", QString::fromStdString(FullRelease->ArtistCredit()->NameCreditList()->Item(0)->Artist()->Name()));

                                    cddbList->append(trackInfo);
                                    //Track->ArtistCredit()->NameCreditList()->Item(0)->Name();*/
                                    if (Recording) {
                                        info.tracks.append(QString::fromStdString(Recording->Title()));
                                    } else {
                                        info.tracks.append(tr("Track %n", nullptr, info.tracks.count() + 1));
                                    }
                                }

                                QEventLoop* eventLoop = new QEventLoop;
                                connect(artReply, &QNetworkReply::finished, [=, &info] {
                                    if (artReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200) {
                                        info.art = QImage::fromData(artReply->readAll());
                                    }
                                    eventLoop->exit();
                                });
                                eventLoop->exec();

                                info.valid = true;
                                return info;
                            }
                        }
                    } catch (...) {
                    }
                    return CddbInfo();
                }))->then([=](CddbInfo info) {
                    if (info.valid) {
                        title = info.title;
                        trackData = info.tracks;
                        art = info.art;
                        emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
                    }
                    emit queryingCddb(false);
                });
            }

            changeUiPane(2);
        }

        emit dataChanged(index(0, 0), index(rowCount(), columnCount()));

        delete info;
    });
}

QString CdModel::cdTitle() {
    return title;
}

QImage CdModel::getArt() {
    return art;
}

void CdModel::eject() {
    changeUiPane(1);
    QDBusInterface cdDriveInterface("org.freedesktop.UDisks2", cdDrivePath.path(), "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus());
    cdDriveInterface.call(QDBus::NoBlock, "Eject", QVariantMap());
}
