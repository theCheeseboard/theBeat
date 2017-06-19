#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QAbstractListModel>
#include <phonon/MediaSource>
#include <phonon/MediaObject>
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

private slots:
    void mediaChanged(Phonon::MediaSource source);

private:
    MediaObject* mediaObj;

    QList<MediaSource> sources;
    int currentPlayingItem = -1;

    QFileIconProvider fileIconProvider;
};

Q_DECLARE_METATYPE(MediaSource)

#endif // PLAYLISTMODEL_H
