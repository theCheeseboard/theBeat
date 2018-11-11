#include "tagcache.h"

QMap<QString, TagLib::Tag*> TagCache::tags;
QMap<QString, TagLib::AudioProperties*> TagCache::audioProperties;

TagCache::TagCache(QObject *parent) : QObject(parent)
{

}

TagLib::Tag* TagCache::getTag(QString filename) {
    if (tags.contains(filename)) {
        return tags.value(filename);
    } else {
        return cache(filename)->tag();
    }
}

TagLib::AudioProperties* TagCache::getAudioProperties(QString filename) {
    if (audioProperties.contains(filename)) {
        return audioProperties.value(filename);
    } else {
        return cache(filename)->audioProperties();
    }
}

TagLib::FileRef* TagCache::cache(QString filename) {
    //FileRef doesn't get deleted so tag doesn't get deleted
    TagLib::FileRef* tag = new TagLib::FileRef(filename.toUtf8());
    tags.insert(filename, tag->tag());
    audioProperties.insert(filename, tag->audioProperties());
    return tag;
}
