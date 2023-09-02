project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets DBus Network)
find_package(libcontemporary)

set(SOURCES
        lastfmapiservice.cpp
        scrobbleservice.cpp
        settings/lastfmloginpopover.cpp settings/lastfmloginpopover.ui
        settings/lastfmsettingspane.cpp settings/lastfmsettingspane.ui
        plugin.cpp
    )

set(HEADERS
        lastfmapiservice.h
        scrobbleservice.h
        settings/lastfmloginpopover.h
        settings/lastfmsettingspane.h
        plugin.h
    )

add_library(plugin-lastfm SHARED ${SOURCES} ${HEADERS})
target_include_directories(plugin-lastfm PUBLIC ../../libthebeat/)

cntp_init_plugin(thebeat plugin-lastfm 20 lastfm)
cntp_translate(plugin-lastfm)

set_target_properties(plugin-lastfm PROPERTIES
        OUTPUT_NAME lastFmPlugin
        FRAMEWORK FALSE)

target_link_libraries(plugin-lastfm Qt::Widgets Qt::DBus libcontemporary libthebeat)
