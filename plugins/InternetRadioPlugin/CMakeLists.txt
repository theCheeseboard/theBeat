project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets DBus)
find_package(libcontemporary)
cntp_find_pkgconfig(X11 x11)

set(SOURCES
        favouritestationswidget.cpp
        plugin.cpp
        radioinfoclient.cpp
        radiopane.cpp
        stationsearchwidget.cpp
        stationwidget.cpp

        favouritestationswidget.ui
        radiopane.ui
        stationsearchwidget.ui
        stationwidget.ui)

set(HEADERS
        favouritestationswidget.h
        plugin.h
        radioinfoclient.h
        radiopane.h
        stationsearchwidget.h
        stationwidget.h)

add_library(plugin-internetradio SHARED ${SOURCES} ${HEADERS})
target_include_directories(plugin-internetradio PUBLIC ../../libthebeat/)

cntp_init_plugin(thebeat plugin-internetradio 17 internetradioplugin)
cntp_translate(plugin-internetradio)

set_target_properties(plugin-internetradio PROPERTIES
        OUTPUT_NAME internetRadioPlugin
        FRAMEWORK FALSE)

target_link_libraries(plugin-internetradio Qt6::Widgets Qt6::DBus libcontemporary libthebeat)