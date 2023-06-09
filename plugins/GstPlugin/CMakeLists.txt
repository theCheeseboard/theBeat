project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets DBus)
find_package(libcontemporary)
find_package(libthefrisbee)
pkg_check_modules(gstreamer IMPORTED_TARGET REQUIRED gstreamer-1.0)

set(SOURCES
        mediaitem/gstmediaitem.cpp
        mediaitem/gsturlhandler.cpp
        mediaitem/gstcdplayback.cpp
        cdwidget.cpp cdwidget.ui
        cdmonitor.cpp
        plugin.cpp
    )

set(HEADERS
        mediaitem/gstmediaitem.h
        mediaitem/gsturlhandler.h
        mediaitem/gstcdplayback.h
        cdwidget.h
        cdmonitor.h
        plugin.h
    )

add_library(plugin-gst SHARED ${SOURCES} ${HEADERS})
target_include_directories(plugin-gst PUBLIC ../../libthebeat/)

cntp_init_plugin(thebeat plugin-gst 20 gst)
cntp_translate(plugin-gst)

set_target_properties(plugin-gst PROPERTIES
        OUTPUT_NAME gstPlugin
        FRAMEWORK FALSE)

target_link_libraries(plugin-gst Qt::Widgets Qt::DBus PkgConfig::gstreamer libcontemporary libthefrisbee libthebeat)
