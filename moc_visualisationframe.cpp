/****************************************************************************
** Meta object code from reading C++ file 'visualisationframe.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "visualisationframe.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QVector>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'visualisationframe.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_VisualisationFrame_t {
    QByteArrayData data[10];
    char stringdata0[141];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_VisualisationFrame_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_VisualisationFrame_t qt_meta_stringdata_VisualisationFrame = {
    {
QT_MOC_LITERAL(0, 0, 18), // "VisualisationFrame"
QT_MOC_LITERAL(1, 19, 24), // "visualisationRateChanged"
QT_MOC_LITERAL(2, 44, 0), // ""
QT_MOC_LITERAL(3, 45, 4), // "rate"
QT_MOC_LITERAL(4, 50, 16), // "setVisualisation"
QT_MOC_LITERAL(5, 67, 15), // "QVector<qint16>"
QT_MOC_LITERAL(6, 83, 13), // "visualisation"
QT_MOC_LITERAL(7, 97, 20), // "setVisualisationType"
QT_MOC_LITERAL(8, 118, 17), // "visualisationType"
QT_MOC_LITERAL(9, 136, 4) // "type"

    },
    "VisualisationFrame\0visualisationRateChanged\0"
    "\0rate\0setVisualisation\0QVector<qint16>\0"
    "visualisation\0setVisualisationType\0"
    "visualisationType\0type"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_VisualisationFrame[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    1,   32,    2, 0x0a /* Public */,
       7,    1,   35,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 8,    9,

       0        // eod
};

void VisualisationFrame::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        VisualisationFrame *_t = static_cast<VisualisationFrame *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->visualisationRateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->setVisualisation((*reinterpret_cast< QVector<qint16>(*)>(_a[1]))); break;
        case 2: _t->setVisualisationType((*reinterpret_cast< visualisationType(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QVector<qint16> >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (VisualisationFrame::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&VisualisationFrame::visualisationRateChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject VisualisationFrame::staticMetaObject = {
    { &QFrame::staticMetaObject, qt_meta_stringdata_VisualisationFrame.data,
      qt_meta_data_VisualisationFrame,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *VisualisationFrame::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VisualisationFrame::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_VisualisationFrame.stringdata0))
        return static_cast<void*>(this);
    return QFrame::qt_metacast(_clname);
}

int VisualisationFrame::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void VisualisationFrame::visualisationRateChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
