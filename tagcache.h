#ifndef TAGCACHE_H
#define TAGCACHE_H

#include <QObject>
#include <QMap>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <QImage>
#include <tpromise.h>

class TagCache : public QObject
{
        Q_OBJECT
    public:
        explicit TagCache(QObject *parent = nullptr);

        static TagLib::Tag* getTag(QString filename);
        static TagLib::AudioProperties* getAudioProperties(QString filename);
        static tPromise<QImage>* getAlbumArt(QString filename);

    signals:

    public slots:

    private:
        static QMap<QString, TagLib::Tag*> tags;
        static QMap<QString, TagLib::AudioProperties*> audioProperties;

        static TagLib::FileRef* cache(QString filename);
};

#endif // TAGCACHE_H
