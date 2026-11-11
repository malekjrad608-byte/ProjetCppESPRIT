QT += testlib pdf
CONFIG += testcase
include(../ANIWORLD2.pro)
TARGET = service_integration_tests
SOURCES -= main.cpp
INCLUDEPATH += $$PWD/..
VPATH += $$PWD/..
SOURCES += $$PWD/service_integration_tests.cpp
