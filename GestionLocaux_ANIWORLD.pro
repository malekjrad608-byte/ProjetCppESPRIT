QT       += core gui widgets svg sql
CONFIG   += c++17
TEMPLATE  = app
TARGET    = GestionLocaux_ANIWORLD

# Evite les problemes d'accents avec MSVC
msvc: QMAKE_CXXFLAGS += /utf-8

INCLUDEPATH += src

SOURCES += \
    main.cpp \
    src/local.cpp \
    src/localrepository.cpp \
    src/connection.cpp \
    src/icons.cpp \
    src/hoverglow.cpp \
    src/toast.cpp \
    src/pawburst.cpp \
    src/spinlogo.cpp \
    src/roomcard.cpp \
    src/cardgrid.cpp \
    src/detailpanel.cpp \
    src/statsdialog.cpp \
    src/alerts.cpp \
    src/alertsdialog.cpp \
    src/clinicmap.cpp \
    src/confirmdialog.cpp \
    src/localdialog.cpp \
    src/pdfreport.cpp \
    src/plandialog.cpp \
    src/smartassign.cpp \
    src/smartsearch.cpp \
    src/smartassigndialog.cpp \
    src/pagelocaux.cpp \
    src/mainwindow.cpp

HEADERS += \
    src/theme.h \
    src/local.h \
    src/localrepository.h \
    src/connection.h \
    src/icons.h \
    src/hoverglow.h \
    src/toast.h \
    src/pawburst.h \
    src/spinlogo.h \
    src/roomcard.h \
    src/cardgrid.h \
    src/detailpanel.h \
    src/statsdialog.h \
    src/alerts.h \
    src/alertsdialog.h \
    src/clinicmap.h \
    src/confirmdialog.h \
    src/localdialog.h \
    src/pdfreport.h \
    src/plandialog.h \
    src/smartassign.h \
    src/smartsearch.h \
    src/smartassigndialog.h \
    src/pagelocaux.h \
    src/mainwindow.h

RESOURCES += resources.qrc

win32: RC_ICONS = resources/images/aniworld.ico
