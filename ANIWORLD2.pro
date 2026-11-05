QT += core gui widgets sql printsupport svg network

CONFIG += c++17
CONFIG += console      # qDebug() visible dans la console (Atelier Connexion)

TARGET = ANIWORLD2
TEMPLATE = app

SOURCES += \
    connection.cpp \
    main.cpp \
    mainwindow.cpp \
    employe.cpp \
    planning.cpp \
    gestionemployes.cpp \
    loginvisuals.cpp \
    medicalwindow.cpp \
    recordcard.cpp \
    medicalrecord.cpp \
    lookupdata.cpp \
    statscurvewidget.cpp \
    medicalrecorddialog.cpp \
    medicalrecorddetailsdialog.cpp \
    motion.cpp \
    fieldvalidator.cpp \
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
    connection.h \
    mainwindow.h \
    employe.h \
    planning.h \
    gestionemployes.h \
    loginvisuals.h \
    medicalwindow.h \
    recordcard.h \
    medicalrecord.h \
    lookupdata.h \
    statscurvewidget.h \
    medicalrecorddialog.h \
    medicalrecorddetailsdialog.h \
    motion.h \
    fieldvalidator.h \
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
    medicalwindow.ui

RESOURCES += \
    resources.qrc \
    medical.qrc
