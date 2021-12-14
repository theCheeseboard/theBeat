#ifndef AVFOUNDATIONMEDIAITEM_H
#define AVFOUNDATIONMEDIAITEM_H

#include <mediaitem.h>

struct AvFoundationMediaItemPrivate;
class AvFoundationMediaItem : public MediaItem
{
    Q_OBJECT
public:
    explicit AvFoundationMediaItem(QUrl url);
    ~AvFoundationMediaItem();

signals:

private:
    AvFoundationMediaItemPrivate* d;

    // MediaItem interface
public:
    void play();
    void pause();
    void stop();
    void seek(quint64 ms);
    quint64 elapsed();
    quint64 duration();
    QString title();
    QStringList authors();
    QString album();
    QImage albumArt();
    QVariant metadata(QString key);
};

#endif // AVFOUNDATIONMEDIAITEM_H
