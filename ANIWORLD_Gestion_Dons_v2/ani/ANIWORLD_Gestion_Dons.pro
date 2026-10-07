QT += core gui widgets printsupport sql
CONFIG += c++17
TEMPLATE = app
TARGET = ANIWORLD_Gestion_Dons

SOURCES += \
    main.cpp \
    donationwindow.cpp \
    loginwindow.cpp \
    loginvisuals.cpp

HEADERS += \
    donationwindow.h \
    loginwindow.h \
    loginvisuals.h

FORMS += \
    donation.ui

RESOURCES += \
    resources.qrc

win32:CONFIG += windows
