/****************************************************************************
** Meta object code from reading C++ file 'playlistmodel.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "playlistmodel.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'playlistmodel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_PlaylistModel_t {
    QByteArrayData data[17];
    char stringdata0[171];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PlaylistModel_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PlaylistModel_t qt_meta_stringdata_PlaylistModel = {
    {
QT_MOC_LITERAL(0, 0, 13), // "PlaylistModel"
QT_MOC_LITERAL(1, 14, 6), // "append"
QT_MOC_LITERAL(2, 21, 0), // ""
QT_MOC_LITERAL(3, 22, 11), // "MediaSource"
QT_MOC_LITERAL(4, 34, 6), // "source"
QT_MOC_LITERAL(5, 41, 14), // "appendPlaylist"
QT_MOC_LITERAL(6, 56, 4), // "path"
QT_MOC_LITERAL(7, 61, 18), // "enqueueAndPlayNext"
QT_MOC_LITERAL(8, 80, 11), // "enqueueNext"
QT_MOC_LITERAL(9, 92, 8), // "playItem"
QT_MOC_LITERAL(10, 101, 1), // "i"
QT_MOC_LITERAL(11, 103, 8), // "playNext"
QT_MOC_LITERAL(12, 112, 8), // "skipBack"
QT_MOC_LITERAL(13, 121, 9), // "setRepeat"
QT_MOC_LITERAL(14, 131, 6), // "repeat"
QT_MOC_LITERAL(15, 138, 12), // "mediaChanged"
QT_MOC_LITERAL(16, 151, 19) // "Phonon::MediaSource"

    },
    "PlaylistModel\0append\0\0MediaSource\0"
    "source\0appendPlaylist\0path\0"
    "enqueueAndPlayNext\0enqueueNext\0playItem\0"
    "i\0playNext\0skipBack\0setRepeat\0repeat\0"
    "mediaChanged\0Phonon::MediaSource"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PlaylistModel[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   59,    2, 0x0a /* Public */,
       5,    1,   62,    2, 0x0a /* Public */,
       7,    0,   65,    2, 0x0a /* Public */,
       8,    0,   66,    2, 0x0a /* Public */,
       9,    1,   67,    2, 0x0a /* Public */,
      11,    0,   70,    2, 0x0a /* Public */,
      12,    0,   71,    2, 0x0a /* Public */,
      13,    1,   72,    2, 0x0a /* Public */,
      15,    1,   75,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   10,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   14,
    QMetaType::Void, 0x80000000 | 16,    4,

       0        // eod
};

void PlaylistModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        PlaylistModel *_t = static_cast<PlaylistModel *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->append((*reinterpret_cast< MediaSource(*)>(_a[1]))); break;
        case 1: _t->appendPlaylist((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->enqueueAndPlayNext(); break;
        case 3: _t->enqueueNext(); break;
        case 4: _t->playItem((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->playNext(); break;
        case 6: _t->skipBack(); break;
        case 7: _t->setRepeat((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->mediaChanged((*reinterpret_cast< Phonon::MediaSource(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< MediaSource >(); break;
            }
            break;
        }
    }
}

const QMetaObject PlaylistModel::staticMetaObject = {
    { &QAbstractListModel::staticMetaObject, qt_meta_stringdata_PlaylistModel.data,
      qt_meta_data_PlaylistModel,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *PlaylistModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PlaylistModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PlaylistModel.stringdata0))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int PlaylistModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
