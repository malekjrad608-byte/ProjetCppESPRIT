QT += core gui widgets svg printsupport sql network
CONFIG += c++17
TARGET = dossier_medical
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    recordcard.cpp \
    medicalrecord.cpp \
    lookupdata.cpp \
    statscurvewidget.cpp \
    medicalrecorddialog.cpp \
    medicalrecorddetailsdialog.cpp \
    logindialog.cpp \
    loginvisuals.cpp \
    motion.cpp \
    fieldvalidator.cpp \
    Connection.cpp \
    vaccination.cpp \
    smsservice.cpp \
    appointment.cpp \
    appointmentdialog.cpp \
    animalpdf.cpp \
    healthwatch.cpp \
    healthwatchdialog.cpp \
    reminderdialog.cpp \
    treatmentadvisor.cpp \
    groomingcut.cpp \
    groomingdialog.cpp

HEADERS += \
    mainwindow.h \
    recordcard.h \
    medicalrecord.h \
    lookupdata.h \
    statscurvewidget.h \
    medicalrecorddialog.h \
    medicalrecorddetailsdialog.h \
    logindialog.h \
    loginvisuals.h \
    motion.h \
    fieldvalidator.h \
    Connection.h \
    vaccination.h \
    smsservice.h \
    appointment.h \
    appointmentdialog.h \
    animalpdf.h \
    healthwatch.h \
    healthwatchdialog.h \
    reminderdialog.h \
    treatmentadvisor.h \
    groomingcut.h \
    groomingdialog.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    resources.qrc
