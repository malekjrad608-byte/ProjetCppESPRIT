/****************************************************************************
** Meta object code from reading C++ file 'medicalwidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/medical/medicalwidget.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'medicalwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.7.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSMedicalWidgetENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSMedicalWidgetENDCLASS = QtMocHelpers::stringData(
    "MedicalWidget",
    "onNavigationClicked",
    "",
    "name",
    "onAddRecordClicked",
    "onViewRecordClicked",
    "onEditRecordClicked",
    "onDeleteRecordClicked",
    "onToggleVaccinationClicked",
    "onRefreshClicked",
    "onFilterChanged",
    "onResetFiltersClicked",
    "onShowStatisticsClicked",
    "onExportPdfClicked",
    "onCardClicked",
    "recordId"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSMedicalWidgetENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   86,    2, 0x08,    1 /* Private */,
       4,    0,   89,    2, 0x08,    3 /* Private */,
       5,    0,   90,    2, 0x08,    4 /* Private */,
       6,    0,   91,    2, 0x08,    5 /* Private */,
       7,    0,   92,    2, 0x08,    6 /* Private */,
       8,    0,   93,    2, 0x08,    7 /* Private */,
       9,    0,   94,    2, 0x08,    8 /* Private */,
      10,    0,   95,    2, 0x08,    9 /* Private */,
      11,    0,   96,    2, 0x08,   10 /* Private */,
      12,    0,   97,    2, 0x08,   11 /* Private */,
      13,    0,   98,    2, 0x08,   12 /* Private */,
      14,    1,   99,    2, 0x08,   13 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   15,

       0        // eod
};

Q_CONSTINIT const QMetaObject MedicalWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_CLASSMedicalWidgetENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSMedicalWidgetENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSMedicalWidgetENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MedicalWidget, std::true_type>,
        // method 'onNavigationClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onAddRecordClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onViewRecordClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onEditRecordClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDeleteRecordClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onToggleVaccinationClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRefreshClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onFilterChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onResetFiltersClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onShowStatisticsClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onExportPdfClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCardClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void MedicalWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MedicalWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onNavigationClicked((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->onAddRecordClicked(); break;
        case 2: _t->onViewRecordClicked(); break;
        case 3: _t->onEditRecordClicked(); break;
        case 4: _t->onDeleteRecordClicked(); break;
        case 5: _t->onToggleVaccinationClicked(); break;
        case 6: _t->onRefreshClicked(); break;
        case 7: _t->onFilterChanged(); break;
        case 8: _t->onResetFiltersClicked(); break;
        case 9: _t->onShowStatisticsClicked(); break;
        case 10: _t->onExportPdfClicked(); break;
        case 11: _t->onCardClicked((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *MedicalWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MedicalWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSMedicalWidgetENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MedicalWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 12;
    }
    return _id;
}
QT_WARNING_POP
