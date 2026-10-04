QT += core gui widgets printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = ANIWORLD
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    salle.cpp \
    Sallecard.cpp \
    cage.cpp \
    gestionnairesalles.cpp \
    mapwidget.cpp \
    statsdialog.cpp \
    loginwindow.cpp \
    loginvisuals.cpp

HEADERS += \
    icones.h \
    mainwindow.h \
    salle.h \
    Sallecard.h \
    cage.h \
    gestionnairesalles.h \
    mapwidget.h \
    statsdialog.h \
    loginwindow.h \
    loginvisuals.h

RESOURCES += \
    resources.qrc

DISTFILES += \
    logo.png \
    check.png \
    style.qss \
    login_assets/check.svg
