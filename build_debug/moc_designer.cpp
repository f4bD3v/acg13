/****************************************************************************
** Meta object code from reading C++ file 'designer.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../nori_framework/include/nori/designer.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'designer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Designer[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      15,   10,    9,    9, 0x05,

 // slots: signature, parameters, type, tag, flags
      31,    9,    9,    9, 0x08,
      55,    9,    9,    9, 0x08,
      79,    9,    9,    9, 0x08,
     104,   10,    9,    9, 0x08,
     125,    9,    9,    9, 0x28,
     135,    9,    9,    9, 0x08,
     148,    9,    9,    9, 0x08,
     161,    9,    9,    9, 0x08,
     175,    9,    9,    9, 0x08,
     190,    9,    9,    9, 0x08,
     208,  202,    9,    9, 0x08,
     229,    9,    9,    9, 0x28,

       0        // eod
};

static const char qt_meta_stringdata_Designer[] = {
    "Designer\0\0mesh\0killMesh(Mesh*)\0"
    "on_loadButton_clicked()\0on_saveButton_clicked()\0"
    "on_clearButton_clicked()\0addMesh(const Mesh*)\0"
    "addMesh()\0removeMesh()\0toggleMesh()\0"
    "toggleLight()\0setMeshColor()\0resetMesh()\0"
    "entry\0updateUI(MeshEntry*)\0updateUI()\0"
};

void Designer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Designer *_t = static_cast<Designer *>(_o);
        switch (_id) {
        case 0: _t->killMesh((*reinterpret_cast< Mesh*(*)>(_a[1]))); break;
        case 1: _t->on_loadButton_clicked(); break;
        case 2: _t->on_saveButton_clicked(); break;
        case 3: _t->on_clearButton_clicked(); break;
        case 4: _t->addMesh((*reinterpret_cast< const Mesh*(*)>(_a[1]))); break;
        case 5: _t->addMesh(); break;
        case 6: _t->removeMesh(); break;
        case 7: _t->toggleMesh(); break;
        case 8: _t->toggleLight(); break;
        case 9: _t->setMeshColor(); break;
        case 10: _t->resetMesh(); break;
        case 11: _t->updateUI((*reinterpret_cast< MeshEntry*(*)>(_a[1]))); break;
        case 12: _t->updateUI(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Designer::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Designer::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Designer,
      qt_meta_data_Designer, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Designer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Designer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Designer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Designer))
        return static_cast<void*>(const_cast< Designer*>(this));
    return QWidget::qt_metacast(_clname);
}

int Designer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void Designer::killMesh(Mesh * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
