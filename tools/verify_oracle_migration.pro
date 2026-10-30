QT += core sql
QT -= gui
CONFIG += console c++17
TARGET = verify_oracle_migration
SOURCES += verify_oracle_migration.cpp ../src/database/oracleconnection.cpp
HEADERS += ../src/database/oracleconnection.h
win32:LIBS += -lcrypt32
