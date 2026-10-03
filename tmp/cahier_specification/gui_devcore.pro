QT += core gui widgets
CONFIG += c++17 release
TEMPLATE = app
TARGET = devcore_spec_gui
SOURCES += gui_devcore.cpp ../../src/ui/design.cpp
RESOURCES += ../../resources/resources.qrc
