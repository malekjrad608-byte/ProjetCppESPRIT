QT += core gui widgets sql

CONFIG += c++17

TARGET = ANIWORLD2
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    employe.cpp \
    gestionemployes.cpp \
    loginwindow.cpp \
    loginvisuals.cpp

HEADERS += \
    mainwindow.h \
    employe.h \
    gestionemployes.h \
    loginwindow.h \
    loginvisuals.h

RESOURCES += \
    resources.qrc