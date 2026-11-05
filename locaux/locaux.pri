# Module "Gestion des locaux" (Emna) - a inclure dans le .pro de l'application :
#     include(locaux/locaux.pri)

INCLUDEPATH += $$PWD/src $$PWD/..

SOURCES += \
    $$PWD/src/cardgrid.cpp \
    $$PWD/src/clinicmap.cpp \
    $$PWD/src/detailpanel.cpp \
    $$PWD/src/espacelocaux.cpp \
    $$PWD/src/flatmap.cpp \
    $$PWD/src/hoverglow.cpp \
    $$PWD/src/icons.cpp \
    $$PWD/src/local.cpp \
    $$PWD/src/localform.cpp \
    $$PWD/src/localrepository.cpp \
    $$PWD/src/pagelocaux.cpp \
    $$PWD/src/pdfreport.cpp \
    $$PWD/src/plandialog.cpp \
    $$PWD/src/roomcard.cpp \
    $$PWD/src/smartassign.cpp \
    $$PWD/src/smartassigndialog.cpp \
    $$PWD/src/smartsearch.cpp \
    $$PWD/src/statsdialog.cpp \
    $$PWD/src/toast.cpp

HEADERS += \
    $$PWD/src/cardgrid.h \
    $$PWD/src/clinicmap.h \
    $$PWD/src/detailpanel.h \
    $$PWD/src/espacelocaux.h \
    $$PWD/src/flatmap.h \
    $$PWD/src/hoverglow.h \
    $$PWD/src/icons.h \
    $$PWD/src/local.h \
    $$PWD/src/localform.h \
    $$PWD/src/localrepository.h \
    $$PWD/src/pagelocaux.h \
    $$PWD/src/pdfreport.h \
    $$PWD/src/plandialog.h \
    $$PWD/src/roomcard.h \
    $$PWD/src/smartassign.h \
    $$PWD/src/smartassigndialog.h \
    $$PWD/src/smartsearch.h \
    $$PWD/src/statsdialog.h \
    $$PWD/src/theme.h \
    $$PWD/src/toast.h

RESOURCES += $$PWD/locaux.qrc
