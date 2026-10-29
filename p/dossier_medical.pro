QT += core gui widgets svg printsupport sql
CONFIG += c++17
TARGET = dossier_medical
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    recordcard.cpp \
    medicalrecord.cpp \
    medicalrecorddialog.cpp \
    medicalrecorddetailsdialog.cpp \
    logindialog.cpp \
    loginvisuals.cpp \
    motion.cpp \
    Connection.cpp

HEADERS += \
    mainwindow.h \
    recordcard.h \
    medicalrecord.h \
    medicalrecorddialog.h \
    medicalrecorddetailsdialog.h \
    logindialog.h \
    loginvisuals.h \
    motion.h \
    Connection.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    resources.qrc
