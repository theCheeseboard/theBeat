project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia)
find_package(libcontemporary)
find_package(libthefrisbee)
pkg_check_modules(tag IMPORTED_TARGET REQUIRED taglib)

set(SOURCES
        abstractburnjob.cpp
        burndevice.cpp
        burnjob.cpp
        burnjobmp3.cpp
        burnjobwidget.cpp
        burnpopover.cpp
        drivewatcher.cpp
        fullburnjob.cpp
        plugin.cpp

        burnjobwidget.ui
        burnpopover.ui)

set(HEADERS
        abstractburnjob.h
        burndevice.h
        burnjob.h
        burnjobmp3.h
        burnjobwidget.h
        burnpopover.h
        drivewatcher.h
        fullburnjob.h
        plugin.h)

add_library(plugin-cdrdao SHARED ${SOURCES} ${HEADERS})
target_include_directories(plugin-cdrdao PUBLIC ../../libthebeat/)

cntp_init_plugin(thebeat plugin-cdrdao 20 cdrdaoplugin)
cntp_translate(plugin-cdrdao)

set_target_properties(plugin-cdrdao PROPERTIES
        OUTPUT_NAME cdrdaoPlugin
        FRAMEWORK FALSE)

target_link_libraries(plugin-cdrdao Qt::Widgets Qt::DBus Qt::Multimedia libcontemporary libthefrisbee PkgConfig::tag)
