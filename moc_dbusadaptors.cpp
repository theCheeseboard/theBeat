/****************************************************************************
** Meta object code from reading C++ file 'dbusadaptors.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "dbusadaptors.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dbusadaptors.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MediaPlayer2Adaptor_t {
    QByteArrayData data[15];
    char stringdata0[806];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MediaPlayer2Adaptor_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MediaPlayer2Adaptor_t qt_meta_stringdata_MediaPlayer2Adaptor = {
    {
QT_MOC_LITERAL(0, 0, 19), // "MediaPlayer2Adaptor"
QT_MOC_LITERAL(1, 20, 15), // "D-Bus Interface"
QT_MOC_LITERAL(2, 36, 22), // "org.mpris.MediaPlayer2"
QT_MOC_LITERAL(3, 59, 19), // "D-Bus Introspection"
QT_MOC_LITERAL(4, 79, 623), // "  <interface name=\"org.mpris..."
QT_MOC_LITERAL(5, 641, 4), // "Quit"
QT_MOC_LITERAL(6, 646, 0), // ""
QT_MOC_LITERAL(7, 647, 5), // "Raise"
QT_MOC_LITERAL(8, 653, 7), // "CanQuit"
QT_MOC_LITERAL(9, 661, 8), // "CanRaise"
QT_MOC_LITERAL(10, 670, 12), // "DesktopEntry"
QT_MOC_LITERAL(11, 683, 12), // "HasTrackList"
QT_MOC_LITERAL(12, 696, 8), // "Identity"
QT_MOC_LITERAL(13, 705, 18), // "SupportedMimeTypes"
QT_MOC_LITERAL(14, 724, 19) // "SupportedUriSchemes"

    },
    "MediaPlayer2Adaptor\0D-Bus Interface\0"
    "org.mpris.MediaPlayer2\0D-Bus Introspection\0"
    "  <interface name=\"org.mpris.MediaPlayer2\">\n    <property access=\""
    "read\" type=\"b\" name=\"CanQuit\"/>\n    <property access=\"read\" ty"
    "pe=\"b\" name=\"CanRaise\"/>\n    <property access=\"read\" type=\"b\""
    " name=\"HasTrackList\"/>\n    <property access=\"read\" type=\"s\" nam"
    "e=\"Identity\"/>\n    <property access=\"read\" type=\"s\" name=\"Desk"
    "topEntry\"/>\n    <property access=\"read\" type=\"as\" name=\"Support"
    "edMimeTypes\"/>\n    <property access=\"read\" type=\"as\" name=\"Supp"
    "ortedUriSchemes\"/>\n    <signal name=\"bringToFront\"/>\n    <method "
    "name=\"Raise\"/>\n    <method name=\"Quit\"/>\n  </interface>\n\0"
    "Quit\0\0Raise\0CanQuit\0CanRaise\0DesktopEntry\0"
    "HasTrackList\0Identity\0SupportedMimeTypes\0"
    "SupportedUriSchemes"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MediaPlayer2Adaptor[] = {

 // content:
       7,       // revision
       0,       // classname
       2,   14, // classinfo
       2,   18, // methods
       7,   30, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // classinfo: key, value
       1,    2,
       3,    4,

 // slots: name, argc, parameters, tag, flags
       5,    0,   28,    6, 0x0a /* Public */,
       7,    0,   29,    6, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
       8, QMetaType::Bool, 0x00095001,
       9, QMetaType::Bool, 0x00095001,
      10, QMetaType::QString, 0x00095001,
      11, QMetaType::Bool, 0x00095001,
      12, QMetaType::QString, 0x00095001,
      13, QMetaType::QStringList, 0x00095001,
      14, QMetaType::QStringList, 0x00095001,

       0        // eod
};

void MediaPlayer2Adaptor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MediaPlayer2Adaptor *_t = static_cast<MediaPlayer2Adaptor *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->Quit(); break;
        case 1: _t->Raise(); break;
        default: ;
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        MediaPlayer2Adaptor *_t = static_cast<MediaPlayer2Adaptor *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = _t->canQuit(); break;
        case 1: *reinterpret_cast< bool*>(_v) = _t->canRaise(); break;
        case 2: *reinterpret_cast< QString*>(_v) = _t->desktopEntry(); break;
        case 3: *reinterpret_cast< bool*>(_v) = _t->hasTrackList(); break;
        case 4: *reinterpret_cast< QString*>(_v) = _t->identity(); break;
        case 5: *reinterpret_cast< QStringList*>(_v) = _t->supportedMimeTypes(); break;
        case 6: *reinterpret_cast< QStringList*>(_v) = _t->supportedUriSchemes(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
    Q_UNUSED(_a);
}

const QMetaObject MediaPlayer2Adaptor::staticMetaObject = {
    { &QDBusAbstractAdaptor::staticMetaObject, qt_meta_stringdata_MediaPlayer2Adaptor.data,
      qt_meta_data_MediaPlayer2Adaptor,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *MediaPlayer2Adaptor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MediaPlayer2Adaptor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MediaPlayer2Adaptor.stringdata0))
        return static_cast<void*>(this);
    return QDBusAbstractAdaptor::qt_metacast(_clname);
}

int MediaPlayer2Adaptor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDBusAbstractAdaptor::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
#ifndef QT_NO_PROPERTIES
   else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 7;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}
struct qt_meta_stringdata_PlayerAdaptor_t {
    QByteArrayData data[31];
    char stringdata0[1690];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PlayerAdaptor_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PlayerAdaptor_t qt_meta_stringdata_PlayerAdaptor = {
    {
QT_MOC_LITERAL(0, 0, 13), // "PlayerAdaptor"
QT_MOC_LITERAL(1, 14, 15), // "D-Bus Interface"
QT_MOC_LITERAL(2, 30, 29), // "org.mpris.MediaPlayer2.Player"
QT_MOC_LITERAL(3, 60, 19), // "D-Bus Introspection"
QT_MOC_LITERAL(4, 80, 1387), // "  <interface name=\"org.mpris..."
QT_MOC_LITERAL(5, 1331, 6), // "Seeked"
QT_MOC_LITERAL(6, 1338, 0), // ""
QT_MOC_LITERAL(7, 1339, 4), // "time"
QT_MOC_LITERAL(8, 1344, 4), // "Next"
QT_MOC_LITERAL(9, 1349, 5), // "Pause"
QT_MOC_LITERAL(10, 1355, 4), // "Play"
QT_MOC_LITERAL(11, 1360, 9), // "PlayPause"
QT_MOC_LITERAL(12, 1370, 8), // "Previous"
QT_MOC_LITERAL(13, 1379, 4), // "Seek"
QT_MOC_LITERAL(14, 1384, 8), // "position"
QT_MOC_LITERAL(15, 1393, 11), // "SetPosition"
QT_MOC_LITERAL(16, 1405, 15), // "QDBusObjectPath"
QT_MOC_LITERAL(17, 1421, 5), // "track"
QT_MOC_LITERAL(18, 1427, 4), // "Stop"
QT_MOC_LITERAL(19, 1432, 10), // "CanControl"
QT_MOC_LITERAL(20, 1443, 9), // "CanGoNext"
QT_MOC_LITERAL(21, 1453, 13), // "CanGoPrevious"
QT_MOC_LITERAL(22, 1467, 8), // "CanPause"
QT_MOC_LITERAL(23, 1476, 7), // "CanPlay"
QT_MOC_LITERAL(24, 1484, 11), // "MaximumRate"
QT_MOC_LITERAL(25, 1496, 8), // "Metadata"
QT_MOC_LITERAL(26, 1505, 11), // "MinimumRate"
QT_MOC_LITERAL(27, 1517, 14), // "PlaybackStatus"
QT_MOC_LITERAL(28, 1532, 8), // "Position"
QT_MOC_LITERAL(29, 1541, 4), // "Rate"
QT_MOC_LITERAL(30, 1546, 6) // "Volume"

    },
    "PlayerAdaptor\0D-Bus Interface\0"
    "org.mpris.MediaPlayer2.Player\0"
    "D-Bus Introspection\0"
    "  <interface name=\"org.mpris.MediaPlayer2.Player\">\n    <property ac"
    "cess=\"read\" type=\"s\" name=\"PlaybackStatus\"/>\n    <property acce"
    "ss=\"read\" type=\"a{sv}\" name=\"Metadata\">\n      <annotation value"
    "=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName\"/>\n    </pro"
    "perty>\n    <property access=\"read\" type=\"d\" name=\"Rate\"/>\n    "
    "<property access=\"read\" type=\"d\" name=\"MaximumRate\"/>\n    <prop"
    "erty access=\"read\" type=\"d\" name=\"MinimumRate\"/>\n    <property "
    "access=\"read\" type=\"x\" name=\"Position\"/>\n    <property access=\""
    "read\" type=\"d\" name=\"Volume\"/>\n    <property access=\"read\" typ"
    "e=\"b\" name=\"CanControl\"/>\n    <property access=\"read\" type=\"b\""
    " name=\"CanPlay\"/>\n    <property access=\"read\" type=\"b\" name=\"C"
    "anPause\"/>\n    <property access=\"read\" type=\"b\" name=\"CanGoPrev"
    "ious\"/>\n    <property access=\"read\" type=\"b\" name=\"CanGoNext\"/"
    ">\n    <method name=\"Next\"/>\n    <method name=\"Previous\"/>\n    <"
    "method name=\"Pause\"/>\n    <method name=\"PlayPause\"/>\n    <method"
    " name=\"Stop\"/>\n    <method name=\"Play\"/>\n    <method type=\"ox\""
    " name=\"SetPosition\"/>\n    <method type=\"x\" name=\"Seek\"/>\n    <"
    "signal name=\"Seeked\">\n      <arg name=\"time\" type=\"x\" direction"
    "=\"out\"/>\n      <annotation name=\"org.qtproject.QtDBus.QtTypeName.O"
    "ut1\" value=\"qint64\"/>\n    </signal>  </interface>\n\0"
    "Seeked\0\0time\0Next\0Pause\0Play\0PlayPause\0"
    "Previous\0Seek\0position\0SetPosition\0"
    "QDBusObjectPath\0track\0Stop\0CanControl\0"
    "CanGoNext\0CanGoPrevious\0CanPause\0"
    "CanPlay\0MaximumRate\0Metadata\0MinimumRate\0"
    "PlaybackStatus\0Position\0Rate\0Volume"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PlayerAdaptor[] = {

 // content:
       7,       // revision
       0,       // classname
       2,   14, // classinfo
       9,   18, // methods
      12,   80, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // classinfo: key, value
       1,    2,
       3,    4,

 // signals: name, argc, parameters, tag, flags
       5,    1,   63,    6, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    0,   66,    6, 0x0a /* Public */,
       9,    0,   67,    6, 0x0a /* Public */,
      10,    0,   68,    6, 0x0a /* Public */,
      11,    0,   69,    6, 0x0a /* Public */,
      12,    0,   70,    6, 0x0a /* Public */,
      13,    1,   71,    6, 0x0a /* Public */,
      15,    2,   74,    6, 0x0a /* Public */,
      18,    0,   79,    6, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::LongLong,    7,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::LongLong,   14,
    QMetaType::Void, 0x80000000 | 16, QMetaType::LongLong,   17,   14,
    QMetaType::Void,

 // properties: name, type, flags
      19, QMetaType::Bool, 0x00095001,
      20, QMetaType::Bool, 0x00095001,
      21, QMetaType::Bool, 0x00095001,
      22, QMetaType::Bool, 0x00095001,
      23, QMetaType::Bool, 0x00095001,
      24, QMetaType::Double, 0x00095001,
      25, QMetaType::QVariantMap, 0x00095001,
      26, QMetaType::Double, 0x00095001,
      27, QMetaType::QString, 0x00095001,
      28, QMetaType::LongLong, 0x00095001,
      29, QMetaType::Double, 0x00095001,
      30, QMetaType::Double, 0x00095001,

       0        // eod
};

void PlayerAdaptor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        PlayerAdaptor *_t = static_cast<PlayerAdaptor *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->Seeked((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 1: _t->Next(); break;
        case 2: _t->Pause(); break;
        case 3: _t->Play(); break;
        case 4: _t->PlayPause(); break;
        case 5: _t->Previous(); break;
        case 6: _t->Seek((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 7: _t->SetPosition((*reinterpret_cast< QDBusObjectPath(*)>(_a[1])),(*reinterpret_cast< qlonglong(*)>(_a[2]))); break;
        case 8: _t->Stop(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 7:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QDBusObjectPath >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (PlayerAdaptor::*_t)(qint64 );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&PlayerAdaptor::Seeked)) {
                *result = 0;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        PlayerAdaptor *_t = static_cast<PlayerAdaptor *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = _t->canControl(); break;
        case 1: *reinterpret_cast< bool*>(_v) = _t->canGoNext(); break;
        case 2: *reinterpret_cast< bool*>(_v) = _t->canGoPrevious(); break;
        case 3: *reinterpret_cast< bool*>(_v) = _t->canPause(); break;
        case 4: *reinterpret_cast< bool*>(_v) = _t->canPlay(); break;
        case 5: *reinterpret_cast< double*>(_v) = _t->maximumRate(); break;
        case 6: *reinterpret_cast< QVariantMap*>(_v) = _t->metadata(); break;
        case 7: *reinterpret_cast< double*>(_v) = _t->minimumRate(); break;
        case 8: *reinterpret_cast< QString*>(_v) = _t->playbackStatus(); break;
        case 9: *reinterpret_cast< qlonglong*>(_v) = _t->position(); break;
        case 10: *reinterpret_cast< double*>(_v) = _t->rate(); break;
        case 11: *reinterpret_cast< double*>(_v) = _t->volume(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

const QMetaObject PlayerAdaptor::staticMetaObject = {
    { &QDBusAbstractAdaptor::staticMetaObject, qt_meta_stringdata_PlayerAdaptor.data,
      qt_meta_data_PlayerAdaptor,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *PlayerAdaptor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PlayerAdaptor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PlayerAdaptor.stringdata0))
        return static_cast<void*>(this);
    return QDBusAbstractAdaptor::qt_metacast(_clname);
}

int PlayerAdaptor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDBusAbstractAdaptor::qt_metacall(_c, _id, _a);
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
#ifndef QT_NO_PROPERTIES
   else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 12;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 12;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 12;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 12;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 12;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void PlayerAdaptor::Seeked(qint64 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
