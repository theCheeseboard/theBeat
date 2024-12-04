project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Concurrent)
find_package(libcontemporary REQUIRED)

set(SOURCES
        burn/cdtextcrcgenerator.cpp
        burn/cdtextgenerator.cpp
        burn/winburndaoimage.cpp
        burn/winburnjob.cpp
        burn/winburnjobwidget.cpp
        burn/winburnmanager.cpp
        burn/winburnpopover.cpp
        burn/winburnprovider.cpp
        plugin.cpp
        smtcintegration.cpp

        burn/winburnjobwidget.ui
        burn/winburnpopover.ui
    )

set(HEADERS
        burn/cdtextcrcgenerator.h
        burn/cdtextgenerator.h
        burn/daoformatlocker.h
        burn/winburndaoimage.h
        burn/winburnjob.h
        burn/winburnjobwidget.h
        burn/winburnmanager.h
        burn/winburnpopover.h
        burn/winburnprovider.h
        plugin.h
        smtcintegration.h)

add_library(plugin-winintegration SHARED ${SOURCES} ${HEADERS})
target_include_directories(plugin-winintegration PUBLIC ../../libthebeat/ ${INSTALL_DIR}/Include)

qt_add_qml_module(plugin-winintegration
    URI com.vicr123.thebeat.plugin.winintegration
    VERSION 1.0
    QML_FILES
        WinCdPane.qml
    NO_CACHEGEN
)

cntp_init_plugin(thebeat plugin-winintegration 20 linuxintegration)
cntp_translate(plugin-winintegration)

set_target_properties(plugin-winintegration PROPERTIES
        OUTPUT_NAME winIntegration
        FRAMEWORK FALSE)

target_link_libraries(plugin-winintegration PRIVATE Qt::Widgets Qt::DBus Qt::Concurrent libcontemporary libthebeat User32 windowsapp comsuppwd shlwapi)
