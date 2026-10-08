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
    loginvisuals.cpp \
    service/database/databasemanager.cpp \
    service/models/service.cpp \
    service/models/serviceanalytics.cpp \
    service/ui/design.cpp \
    service/ui/motion.cpp \
    service/ui/servicereport.cpp \
    service/widgets/animatedbutton.cpp \
    service/widgets/animatedpanel.cpp \
    service/widgets/dashboardwidget.cpp \
    service/widgets/servicecard.cpp \
    service/widgets/servicewidget.cpp \
    service/dialogs/servicedialog.cpp \
    service/stock/stockwidget.cpp \
    dossiers/medicalrecord.cpp \
    dossiers/recordcard.cpp \
    dossiers/medicalrecorddialog.cpp \
    dossiers/medicalrecorddetailsdialog.cpp \
    dossiers/dossierspage.cpp

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
    loginvisuals.h \
    service/database/databasemanager.h \
    service/models/service.h \
    service/models/serviceanalytics.h \
    service/ui/design.h \
    service/ui/motion.h \
    service/ui/servicereport.h \
    service/widgets/animatedbutton.h \
    service/widgets/animatedpanel.h \
    service/widgets/dashboardwidget.h \
    service/widgets/servicecard.h \
    service/widgets/servicewidget.h \
    service/dialogs/servicedialog.h \
    service/stock/stockwidget.h \
    dossiers/medicalrecord.h \
    dossiers/recordcard.h \
    dossiers/medicalrecorddialog.h \
    dossiers/medicalrecorddetailsdialog.h \
    dossiers/dossierspage.h

FORMS += \
    dossiers/dossierspage.ui

RESOURCES += \
    resources.qrc