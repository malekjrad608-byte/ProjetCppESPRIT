SOURCES += $$PWD/src/mainwindow.cpp \
    $$PWD/src/stock/stockwidget.cpp \
    $$PWD/src/appcontroller.cpp \
    $$PWD/src/login/loginwindow.cpp \
    $$PWD/src/login/loginvisuals.cpp \
    $$PWD/src/database/databasemanager.cpp \
    $$PWD/src/models/service.cpp \
    $$PWD/src/dialogs/servicedialog.cpp \
    $$PWD/src/widgets/servicewidget.cpp \
    $$PWD/src/widgets/dashboardwidget.cpp \
    $$PWD/src/ui/design.cpp \
    $$PWD/src/models/serviceanalytics.cpp \
    $$PWD/src/ui/servicereport.cpp \
    $$PWD/src/widgets/servicecard.cpp \
    $$PWD/src/widgets/animatedlogo.cpp \
    $$PWD/src/widgets/animatedbutton.cpp \
    $$PWD/src/ui/motion.cpp \
    $$PWD/src/widgets/animatedpanel.cpp
HEADERS += $$PWD/src/mainwindow.h \
    $$PWD/src/stock/stockwidget.h \
    $$PWD/src/appcontroller.h \
    $$PWD/src/login/loginwindow.h \
    $$PWD/src/login/loginvisuals.h \
    $$PWD/src/database/databasemanager.h \
    $$PWD/src/models/service.h \
    $$PWD/src/dialogs/servicedialog.h \
    $$PWD/src/widgets/servicewidget.h \
    $$PWD/src/widgets/dashboardwidget.h \
    $$PWD/src/ui/design.h \
    $$PWD/src/models/serviceanalytics.h \
    $$PWD/src/ui/servicereport.h \
    $$PWD/src/widgets/servicecard.h \
    $$PWD/src/widgets/animatedlogo.h \
    $$PWD/src/widgets/animatedbutton.h \
    $$PWD/src/ui/motion.h \
    $$PWD/src/widgets/animatedpanel.h
RESOURCES += $$PWD/resources/resources.qrc

SOURCES += $$PWD/src/ui/invoice.cpp


SOURCES +=  $$PWD/src/dialogs/serviceplanningdialog.cpp
HEADERS +=  $$PWD/src/dialogs/serviceplanningdialog.h
HEADERS += $$PWD/src/ui/invoice.h
QT += pdf network
SOURCES += $$PWD/src/auth/accounts.cpp \
    $$PWD/src/employees/employe.cpp \
    $$PWD/src/employees/gestionemployes.cpp
HEADERS += $$PWD/src/auth/accounts.h \
    $$PWD/src/employees/employe.h \
    $$PWD/src/employees/gestionemployes.h

QT += svg

SOURCES += $$PWD/src/locaux/alerts.cpp \
    $$PWD/src/locaux/alertsdialog.cpp \
    $$PWD/src/locaux/cardgrid.cpp \
    $$PWD/src/locaux/clinicmap.cpp \
    $$PWD/src/locaux/confirmdialog.cpp \
    $$PWD/src/locaux/detailpanel.cpp \
    $$PWD/src/locaux/hoverglow.cpp \
    $$PWD/src/locaux/icons.cpp \
    $$PWD/src/locaux/local.cpp \
    $$PWD/src/locaux/localdialog.cpp \
    $$PWD/src/locaux/localrepository.cpp \
    $$PWD/src/locaux/pagelocaux.cpp \
    $$PWD/src/locaux/pdfreport.cpp \
    $$PWD/src/locaux/plandialog.cpp \
    $$PWD/src/locaux/roomcard.cpp \
    $$PWD/src/locaux/smartassign.cpp \
    $$PWD/src/locaux/smartassigndialog.cpp \
    $$PWD/src/locaux/smartsearch.cpp \
    $$PWD/src/locaux/statsdialog.cpp \
    $$PWD/src/locaux/toast.cpp

HEADERS += $$PWD/src/locaux/alerts.h \
    $$PWD/src/locaux/alertsdialog.h \
    $$PWD/src/locaux/cardgrid.h \
    $$PWD/src/locaux/clinicmap.h \
    $$PWD/src/locaux/confirmdialog.h \
    $$PWD/src/locaux/detailpanel.h \
    $$PWD/src/locaux/hoverglow.h \
    $$PWD/src/locaux/icons.h \
    $$PWD/src/locaux/local.h \
    $$PWD/src/locaux/localdialog.h \
    $$PWD/src/locaux/localrepository.h \
    $$PWD/src/locaux/pagelocaux.h \
    $$PWD/src/locaux/pdfreport.h \
    $$PWD/src/locaux/plandialog.h \
    $$PWD/src/locaux/roomcard.h \
    $$PWD/src/locaux/smartassign.h \
    $$PWD/src/locaux/smartassigndialog.h \
    $$PWD/src/locaux/smartsearch.h \
    $$PWD/src/locaux/statsdialog.h \
    $$PWD/src/locaux/theme.h \
    $$PWD/src/locaux/toast.h

SOURCES += $$PWD/src/medical/medicalrecord.cpp
HEADERS += $$PWD/src/medical/medicalrecord.h
SOURCES += $$PWD/src/medical/medicalrecorddialog.cpp
HEADERS += $$PWD/src/medical/medicalrecorddialog.h
SOURCES += $$PWD/src/medical/medicalrecorddetailsdialog.cpp
HEADERS += $$PWD/src/medical/medicalrecorddetailsdialog.h
SOURCES += $$PWD/src/medical/recordcard.cpp
HEADERS += $$PWD/src/medical/recordcard.h


SOURCES += $$PWD/src/medical/medicalwidget.cpp
HEADERS += $$PWD/src/medical/medicalwidget.h
SOURCES += $$PWD/src/medical/medicalexamples.cpp

FORMS += $$PWD/src/medical/medicalwidget.ui

SOURCES += $$PWD/src/database/oracleconnection.cpp
SOURCES += $$PWD/src/database/connection.cpp
HEADERS += $$PWD/src/database/connection.h

HEADERS += $$PWD/src/database/oracleconnection.h

HEADERS += $$PWD/src/ui/localchoice.h
HEADERS += $$PWD/src/ui/relationchoices.h
win32:LIBS += -lcrypt32

SOURCES += $$PWD/src/models/employeequeries.cpp
HEADERS += $$PWD/src/models/employeequeries.h $$PWD/src/models/modelrows.h

SOURCES += $$PWD/src/models/relationmodel.cpp
HEADERS += $$PWD/src/models/relationmodel.h

SOURCES += $$PWD/src/models/invoicemodel.cpp $$PWD/src/stock/stockrecord.cpp
HEADERS += $$PWD/src/models/invoicemodel.h $$PWD/src/stock/stockrecord.h
