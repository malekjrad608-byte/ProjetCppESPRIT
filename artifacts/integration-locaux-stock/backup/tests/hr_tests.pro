QT += core gui sql widgets testlib
CONFIG += c++17 testcase console
CONFIG -= app_bundle
TARGET = hr_tests
include(../src/rh/rh.pri)
SOURCES += hr_tests.cpp ../src/ui/motion.cpp
HEADERS += ../src/ui/motion.h
RESOURCES += ../resources/resources.qrc
