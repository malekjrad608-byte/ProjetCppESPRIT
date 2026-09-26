QT += core gui sql widgets

CONFIG += c++17

TARGET   = gestion_service
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/database/databasemanager.cpp \
    src/models/service.cpp \
    src/models/employee.cpp \
    src/models/schedule.cpp \
    src/dialogs/servicedialog.cpp \
    src/dialogs/employeedialog.cpp \
    src/dialogs/scheduledialog.cpp \
    src/widgets/servicewidget.cpp \
    src/widgets/schedulewidget.cpp \
    src/widgets/calendarwidget.cpp \
    src/widgets/dashboardwidget.cpp

HEADERS += \
    src/mainwindow.h \
    src/database/databasemanager.h \
    src/models/service.h \
    src/models/employee.h \
    src/models/schedule.h \
    src/dialogs/servicedialog.h \
    src/dialogs/employeedialog.h \
    src/dialogs/scheduledialog.h \
    src/widgets/servicewidget.h \
    src/widgets/schedulewidget.h \
    src/widgets/calendarwidget.h \
    src/widgets/dashboardwidget.h

RESOURCES += resources/resources.qrc