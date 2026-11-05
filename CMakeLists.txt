
cmake_minimum_required(VERSION 3.19)
project(ANIWORLD LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Svg Sql Network)

qt_standard_project_setup()

qt_add_executable(ANIWORLD
    main.cpp
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui
    recordcard.h
    recordcard.cpp
    medicalrecord.h
    medicalrecord.cpp
    lookupdata.h
    lookupdata.cpp
    statscurvewidget.h
    statscurvewidget.cpp
    medicalrecorddialog.h
    medicalrecorddialog.cpp
    medicalrecorddetailsdialog.h
    medicalrecorddetailsdialog.cpp
    logindialog.h
    logindialog.cpp
    loginvisuals.h
    loginvisuals.cpp
    motion.h
    fieldvalidator.h
    fieldvalidator.cpp
    motion.cpp
    Connection.h
    Connection.cpp
    vaccination.h
    vaccination.cpp
    smsservice.h
    smsservice.cpp
    appointment.h
    appointment.cpp
    appointmentdialog.h
    appointmentdialog.cpp
    animalpdf.h
    animalpdf.cpp
    healthwatch.h
    healthwatch.cpp
    healthwatchdialog.h
    healthwatchdialog.cpp
    reminderdialog.h
    reminderdialog.cpp
    treatmentadvisor.h
    treatmentadvisor.cpp
    groomingcut.h
    groomingcut.cpp
    groomingdialog.h
    groomingdialog.cpp
    resources.qrc
)

target_link_libraries(ANIWORLD
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Svg
        Qt::Sql
        Qt::Network
)

install(TARGETS ANIWORLD
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
