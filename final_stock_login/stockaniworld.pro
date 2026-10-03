QT       += core gui widgets sql

TARGET = stockaniworld
TEMPLATE = app

CONFIG += c++17
CONFIG += strict_c++

SOURCES += \
    main.cpp \
    stockwidget.cpp \
    loginwindow.cpp \
    loginvisuals.cpp \
    mainwindow.cpp

HEADERS += \
    stockwidget.h \
    loginwindow.h \
    loginvisuals.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    resources.qrc
