QT += core gui widgets sql printsupport

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
    loginvisuals.cpp

HEADERS += \
    connection.h \
    mainwindow.h \
    employe.h \
    planning.h \
    gestionemployes.h \
    loginvisuals.h

RESOURCES += \
    resources.qrc
