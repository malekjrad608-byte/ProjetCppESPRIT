# === MODULE SERVICES : classe metier, controleurs et ressources ===
SOURCES += \
    $$PWD/database/databasemanager.cpp \
    $$PWD/database/oracleconnection.cpp \
    $$PWD/dialogs/servicedialog.cpp \
    $$PWD/dialogs/serviceplanningdialog.cpp \
    $$PWD/models/employeequeries.cpp \
    $$PWD/models/invoicemodel.cpp \
    $$PWD/models/relationmodel.cpp \
    $$PWD/models/service.cpp \
    $$PWD/models/serviceanalytics.cpp \
    $$PWD/models/servicereferences.cpp \
    $$PWD/ui/design.cpp \
    $$PWD/ui/invoice.cpp \
    $$PWD/ui/servicemotion.cpp \
    $$PWD/ui/servicereport.cpp \
    $$PWD/widgets/animatedpanel.cpp \
    $$PWD/widgets/serviceanimatedbutton.cpp \
    $$PWD/widgets/servicecard.cpp \
    $$PWD/widgets/servicewidget.cpp

HEADERS += \
    $$PWD/database/databasemanager.h \
    $$PWD/database/oracleconnection.h \
    $$PWD/dialogs/servicedialog.h \
    $$PWD/dialogs/serviceplanningdialog.h \
    $$PWD/models/employeequeries.h \
    $$PWD/models/invoicemodel.h \
    $$PWD/models/modelrows.h \
    $$PWD/models/relationmodel.h \
    $$PWD/models/service.h \
    $$PWD/models/serviceanalytics.h \
    $$PWD/models/servicereferences.h \
    $$PWD/ui/design.h \
    $$PWD/ui/invoice.h \
    $$PWD/ui/localchoice.h \
    $$PWD/ui/relationchoices.h \
    $$PWD/ui/servicemotion.h \
    $$PWD/ui/servicereport.h \
    $$PWD/widgets/animatedpanel.h \
    $$PWD/widgets/serviceanimatedbutton.h \
    $$PWD/widgets/servicecard.h \
    $$PWD/widgets/servicewidget.h

RESOURCES += \
    $$PWD/resources/services_resources.qrc \
    $$PWD/resources/services_designer.qrc

