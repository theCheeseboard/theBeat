#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QAbstractListModel>
#include <phonon/MediaSource>
#include <phonon/MediaObject>
#include <phonon/AudioDataOutput>
#include <QFileIconProvider>

using namespace Phonon;

class PlaylistModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit PlaylistModel(MediaObject* object, QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public slots:
    void append(MediaSource source);

    void enqueueAndPlayNext();

    void enqueueNext();

    void playItem(int i);

    void setRepeat(bool repeat);

private slots:
    void mediaChanged(Phonon::MediaSource source);

private:
    MediaObject* mediaObj;
    AudioDataOutput* dataOut;

    QList<MediaSource> sources;
    int currentPlayingItem = -1;

    QFileIconProvider fileIconProvider;

    bool repeat = false;
};

Q_DECLARE_METATYPE(MediaSource)

#endif // PLAYLISTMODEL_H
