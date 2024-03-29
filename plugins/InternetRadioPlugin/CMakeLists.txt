project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets DBus)
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

set_target_properties(plugin-internetradio PROPERTIES
        OUTPUT_NAME internetRadioPlugin
        FRAMEWORK FALSE
        TRANSLATION_SOURCES "${SOURCES};${HEADERS}"
    )

cntp_init_plugin(thebeat plugin-internetradio 20 internetradioplugin)
cntp_translate(plugin-internetradio)


target_link_libraries(plugin-internetradio Qt::Widgets Qt::DBus libcontemporary libthebeat)
