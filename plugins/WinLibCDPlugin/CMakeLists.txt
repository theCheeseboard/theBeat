cmake_minimum_required(VERSION 3.4.0)

project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets DBus Concurrent)
find_package(libcontemporary)

set(SOURCES
        audiocdplayerthread.cpp
        burn/cdtextcrcgenerator.cpp
        burn/cdtextgenerator.cpp
        burn/winburndaoimage.cpp
        burn/winburnjob.cpp
        burn/winburnjobwidget.cpp
        burn/winburnmanager.cpp
        burn/winburnpopover.cpp
        burn/winburnprovider.cpp
        cdchecker.cpp
        diskwatcher.cpp
        plugin.cpp
        trackinfo.cpp
        wincdmediaitem.cpp

        burn/winburnjobwidget.ui
        burn/winburnpopover.ui
        cdchecker.ui)

set(HEADERS
        audiocdplayerthread.h
        burn/cdtextcrcgenerator.h
        burn/cdtextgenerator.h
        burn/daoformatlocker.h
        burn/winburndaoimage.h
        burn/winburnjob.h
        burn/winburnjobwidget.h
        burn/winburnmanager.h
        burn/winburnpopover.h
        burn/winburnprovider.h
        cdchecker.h
        diskwatcher.h
        plugin.h
        trackinfo.h
        wincdmediaitem.h)

add_library(plugin-winlibcd SHARED ${SOURCES} ${HEADERS})
target_include_directories(plugin-winlibcd PUBLIC ../../libthebeat/ ${CMAKE_CURRENT_LIST_DIR}/cdlib/include)

cntp_init_plugin(thebeat plugin-winlibcd 17 linuxintegration)
cntp_translate(plugin-winlibcd)

set_target_properties(plugin-winlibcd PROPERTIES
        OUTPUT_NAME winLibCdIntegration
        FRAMEWORK FALSE)

target_link_libraries(plugin-winlibcd Qt6::Widgets Qt6::DBus Qt6::Concurrent libcontemporary libthebeat User32 windowsapp comsuppwd shlwapi)