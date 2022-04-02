cmake_minimum_required(VERSION 3.4.0)

project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets DBus)
find_package(libcontemporary)
pkg_check_modules(X11 IMPORTED_TARGET x11)

set(SOURCES
        mprisinstance.cpp
        mprisplayer.cpp
        mpriswrapper.cpp
        nativeevents.cpp
        plugin.cpp)

set(HEADERS
        mprisinstance.h
        mprisplayer.h
        mpriswrapper.h
        nativeevents.h
        plugin.h)

add_library(plugin-linuxintegration SHARED ${SOURCES} ${HEADERS})
target_include_directories(plugin-linuxintegration PUBLIC ../../libthebeat/)

cntp_init_plugin(thebeat plugin-linuxintegration 17 linuxintegration)
cntp_translate(plugin-linuxintegration)

set_target_properties(plugin-linuxintegration PROPERTIES
        OUTPUT_NAME linuxIntegrationPlugin
        FRAMEWORK FALSE)

IF(${X11_FOUND})
    target_link_libraries(plugin-linuxintegration PkgConfig::X11)
    target_compile_definitions(plugin-linuxintegration PRIVATE HAVE_X11)
ENDIF()

target_link_libraries(plugin-linuxintegration Qt6::Widgets Qt6::DBus libcontemporary libthebeat)