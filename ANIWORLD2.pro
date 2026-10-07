QT += core gui widgets sql printsupport

CONFIG += c++17

TARGET = ANIWORLD2
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    employe.cpp \
    gestionemployes.cpp \
    gestionlocaux.cpp \
    salle.cpp \
    Sallecard.cpp \
    cage.cpp \
    gestionnairesalles.cpp \
    mapwidget.cpp \
    statsdialog.cpp \
    loginwindow.cpp \
    loginvisuals.cpp

HEADERS += \
    mainwindow.h \
    employe.h \
    gestionemployes.h \
    gestionlocaux.h \
    salle.h \
    Sallecard.h \
    cage.h \
    gestionnairesalles.h \
    mapwidget.h \
    statsdialog.h \
    icones.h \
    loginwindow.h \
    loginvisuals.h

RESOURCES += \
    resources.qrc