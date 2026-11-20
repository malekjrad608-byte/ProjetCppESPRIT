include(../sources.pri)
QT += core gui widgets sql
CONFIG += c++17 console release
CONFIG -= app_bundle debug
TARGET = populate_employee_planning
SOURCES = populate_employee_planning.cpp
HEADERS =
FORMS =
RESOURCES =
INCLUDEPATH += $$PWD/..
# Reuse the application objects after building build_final in release mode.
app_objects = $$files($$PWD/../build_final/release/*.o)
for(object, app_objects) {
    !contains(object, .*/main\.o$): LIBS += $$object
}
