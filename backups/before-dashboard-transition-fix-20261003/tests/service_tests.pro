QT += core gui sql widgets testlib
CONFIG += c++17 testcase console
CONFIG -= app_bundle
TARGET = service_tests
include(../sources.pri)
SOURCES += service_tests.cpp
