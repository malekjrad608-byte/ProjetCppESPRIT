QT += core gui widgets sql printsupport svg network

# MinGW make can exceed MAX_PATH before resolving relative paths in shadow builds.
win32-g++: QMAKE_PROJECT_DEPTH = 0

CONFIG += c++17
CONFIG += console      # qDebug() visible dans la console (Atelier Connexion)

TARGET = ANIWORLD2
TEMPLATE = app

INCLUDEPATH += .

# === COMMUN : connexion Oracle unique + fenetre principale (navigation) ===
SOURCES += \
    connection.cpp \
    main.cpp \
    mainwindow.cpp \
    loginvisuals.cpp
HEADERS += \
    connection.h \
    mainwindow.h \
    loginvisuals.h
# UN SEUL fichier .ui : toutes les pages (stackedWidget)
FORMS += \
    mainwindow.ui
RESOURCES += \
    resources.qrc

# === MODULE EMPLOYE (Linda) ===
SOURCES += \
    employe.cpp \
    gestionemployes.cpp
HEADERS += \
    employe.h \
    gestionemployes.h

# === MODULE DOSSIERS MEDICAUX (Rawand) ===
SOURCES += \
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
    groomingdialog.h \
    rappelsdb.h
RESOURCES += \
    medical.qrc

# === MODULE LOCAUX (Emna) ===
SOURCES += \
    cardgrid.cpp \
    clinicmap.cpp \
    confirmdialog.cpp \
    detailpanel.cpp \
    espacelocaux.cpp \
    flatmap.cpp \
    hoverglow.cpp \
    icons.cpp \
    local.cpp \
    localdialog.cpp \
    localrepository.cpp \
    pagelocaux.cpp \
    pawburst.cpp \
    pdfreport.cpp \
    plandialog.cpp \
    roomcard.cpp \
    smartassign.cpp \
    smartassigndialog.cpp \
    smartsearch.cpp \
    spinlogo.cpp \
    statsdialog.cpp \
    toast.cpp
HEADERS += \
    cardgrid.h \
    clinicmap.h \
    confirmdialog.h \
    detailpanel.h \
    espacelocaux.h \
    flatmap.h \
    hoverglow.h \
    icons.h \
    local.h \
    localdialog.h \
    localrepository.h \
    pagelocaux.h \
    pawburst.h \
    pdfreport.h \
    plandialog.h \
    roomcard.h \
    smartassign.h \
    smartassigndialog.h \
    smartsearch.h \
    spinlogo.h \
    statsdialog.h \
    theme.h \
    toast.h
RESOURCES += \
    locaux.qrc


# === MODULE SERVICES ===
include(services/services.pri)

# === MODULE STOCK (integre dans la navigation principale ANIWORLD) ===
SOURCES += \
    stockwidget.cpp \
    inventoryfeatures.cpp
HEADERS += \
    stockwidget.h \
    inventoryfeatures.h
