QT += core gui widgets sql printsupport
CONFIG += console
TARGET = stockaniworld
TEMPLATE = app
CONFIG += c++17

SOURCES += \
    connection.cpp \
    main.cpp \
    stock.cpp \
    stockwidget.cpp \
    loginwindow.cpp \
    loginvisuals.cpp \
    inventoryfeatures.cpp

HEADERS += \
    connection.h \
    stock.h \
    stockwidget.h \
    loginwindow.h \
    loginvisuals.h \
    inventoryfeatures.h

RESOURCES += resources.qrc
