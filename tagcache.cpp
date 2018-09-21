#include "tagcache.h"

QMap<QString, TagLib::Tag*> TagCache::tags;

TagCache::TagCache(QObject *parent) : QObject(parent)
{

}

TagLib::Tag* TagCache::getTag(QString filename) {
    if (tags.contains(filename)) {
        return tags.value(filename);
    } else {
        //FileRef doesn't get deleted so tag doesn't get deleted
        TagLib::FileRef* tag = new TagLib::FileRef(filename.toUtf8());
        tags.insert(filename, tag->tag());
        return tag->tag();
    }
}
