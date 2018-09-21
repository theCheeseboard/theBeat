#ifndef TAGCACHE_H
#define TAGCACHE_H

#include <QObject>
#include <QMap>
#include <taglib/fileref.h>
#include <taglib/tag.h>

class TagCache : public QObject
{
        Q_OBJECT
    public:
        explicit TagCache(QObject *parent = nullptr);

        static TagLib::Tag* getTag(QString filename);

    signals:

    public slots:

    private:
        static QMap<QString, TagLib::Tag*> tags;
};

#endif // TAGCACHE_H
