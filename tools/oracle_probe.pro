QT += core sql
QT -= gui
CONFIG += console c++17
TARGET = oracle_probe
SOURCES += oracle_probe.cpp ../src/database/oracleconnection.cpp
HEADERS += ../src/database/oracleconnection.h
win32:LIBS += -lcrypt32
