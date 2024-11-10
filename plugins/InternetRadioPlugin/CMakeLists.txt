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
        stationlistmodel.cpp

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
        stationwidget.h
        stationlistmodel.h)

add_library(plugin-internetradio SHARED ${SOURCES} ${HEADERS})
target_include_directories(plugin-internetradio PUBLIC ../../libthebeat/)

set_target_properties(plugin-internetradio PROPERTIES
        OUTPUT_NAME internetRadioPlugin
        FRAMEWORK FALSE
        TRANSLATION_SOURCES "${SOURCES};${HEADERS}"
    )

cntp_init_plugin(thebeat plugin-internetradio 20 internetradioplugin)
cntp_translate(plugin-internetradio)

qt_add_qml_module(plugin-internetradio
    URI com.vicr123.thebeat.plugin.internetradio
    VERSION 1.0
    QML_FILES
        InternetRadioPane.qml
    NO_CACHEGEN
)

target_link_libraries(plugin-internetradio PUBLIC Qt::Widgets Qt::DBus libcontemporary libthebeat)
