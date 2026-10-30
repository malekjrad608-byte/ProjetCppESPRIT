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
HEADERS += $$PWD/src/ui/invoice.h
QT += pdf
