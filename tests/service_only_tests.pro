TEMPLATE = app
TARGET = service_only_tests
include(../sources.pri)
QT += testlib
CONFIG += testcase console
SOURCES += service_only_tests.cpp
