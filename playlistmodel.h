#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QAbstractListModel>
#include <phonon/MediaSource>
#include <phonon/MediaObject>
#include <phonon/AudioDataOutput>
#include <QFileIconProvider>
#include <QMimeData>
#include <QDir>
#include <phonon/MediaController>
#include <taglib/tag.h>
#include <taglib/fileref.h>

using namespace Phonon;

struct MediaItem {
    MediaItem();
    MediaItem(const MediaSource &source);

    int opticalTrack;

    enum MediaType {
        Source,
        Optical
    };

    MediaSource source;
    MediaType currentType;

    operator MediaSource() const {
        return source;
    }

    bool operator==(const MediaItem &other) {
        if (this->currentType == other.currentType) {
            if (this->currentType == Source) {
                return this->source == other.source;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
};

Q_DECLARE_METATYPE(MediaItem)

class PlaylistModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit PlaylistModel(MediaObject* object, QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList &indexes) const override;

    Qt::DropActions supportedDropActions() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;

public slots:
    void append(MediaSource source);

    void appendPlaylist(QString path);

    void enqueueAndPlayNext();

    void enqueueNext();

    void playItem(int i);

    void playNext();

    void skipBack();

    void setRepeat(bool repeat);

private slots:
    void mediaChanged(Phonon::MediaSource source);

private:

    MediaObject* mediaObj;
    MediaController* controller;
    AudioDataOutput* dataOut;

    QList<MediaItem> sources;
    int currentPlayingItem = -1;

    QFileIconProvider fileIconProvider;

    bool repeat = false;
};

Q_DECLARE_METATYPE(MediaSource)

#endif // PLAYLISTMODEL_H
