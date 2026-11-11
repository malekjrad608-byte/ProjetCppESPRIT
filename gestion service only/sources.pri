QT += core gui widgets sql network svg pdf
CONFIG += c++17
INCLUDEPATH += $$PWD
SOURCES += \
    $$PWD/src/appcontroller.cpp \
    $$PWD/src/auth/accounts.cpp \
    $$PWD/src/database/connection.cpp \
    $$PWD/src/database/databasemanager.cpp \
    $$PWD/src/database/oracleconnection.cpp \
    $$PWD/src/dialogs/servicedialog.cpp \
    $$PWD/src/dialogs/serviceplanningdialog.cpp \
    $$PWD/src/login/loginvisuals.cpp \
    $$PWD/src/login/loginwindow.cpp \
    $$PWD/src/mainwindow.cpp \
    $$PWD/src/models/employeequeries.cpp \
    $$PWD/src/models/invoicemodel.cpp \
    $$PWD/src/models/relationmodel.cpp \
    $$PWD/src/models/service.cpp \
    $$PWD/src/models/serviceanalytics.cpp \
    $$PWD/src/models/servicereferences.cpp \
    $$PWD/src/ui/design.cpp \
    $$PWD/src/ui/invoice.cpp \
    $$PWD/src/ui/motion.cpp \
    $$PWD/src/ui/servicereport.cpp \
    $$PWD/src/widgets/animatedbutton.cpp \
    $$PWD/src/widgets/animatedlogo.cpp \
    $$PWD/src/widgets/animatedpanel.cpp \
    $$PWD/src/widgets/dashboardwidget.cpp \
    $$PWD/src/widgets/servicecard.cpp \
    $$PWD/src/widgets/servicewidget.cpp
HEADERS += \
    $$PWD/src/appcontroller.h \
    $$PWD/src/auth/accounts.h \
    $$PWD/src/database/connection.h \
    $$PWD/src/database/databasemanager.h \
    $$PWD/src/database/oracleconnection.h \
    $$PWD/src/dialogs/servicedialog.h \
    $$PWD/src/dialogs/serviceplanningdialog.h \
    $$PWD/src/login/loginvisuals.h \
    $$PWD/src/login/loginwindow.h \
    $$PWD/src/mainwindow.h \
    $$PWD/src/models/employeequeries.h \
    $$PWD/src/models/invoicemodel.h \
    $$PWD/src/models/modelrows.h \
    $$PWD/src/models/relationmodel.h \
    $$PWD/src/models/service.h \
    $$PWD/src/models/serviceanalytics.h \
    $$PWD/src/models/servicereferences.h \
    $$PWD/src/ui/design.h \
    $$PWD/src/ui/invoice.h \
    $$PWD/src/ui/localchoice.h \
    $$PWD/src/ui/motion.h \
    $$PWD/src/ui/relationchoices.h \
    $$PWD/src/ui/servicereport.h \
    $$PWD/src/widgets/animatedbutton.h \
    $$PWD/src/widgets/animatedlogo.h \
    $$PWD/src/widgets/animatedpanel.h \
    $$PWD/src/widgets/dashboardwidget.h \
    $$PWD/src/widgets/servicecard.h \
    $$PWD/src/widgets/servicewidget.h
FORMS += $$PWD/src/mainwindow.ui
RESOURCES += $$PWD/resources/resources.qrc $$PWD/resources/designer.qrc
win32:LIBS += -lcrypt32
