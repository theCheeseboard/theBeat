project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets DBus)
find_package(libcontemporary)
pkg_check_modules(gstreamer IMPORTED_TARGET REQUIRED gstreamer-1.0)

set(SOURCES
        mediaitem/gstmediaitem.cpp
        mediaitem/gsturlhandler.cpp
        mediaitem/gstcdplayback.cpp
        plugin.cpp
    )

set(HEADERS
        mediaitem/gstmediaitem.h
        mediaitem/gsturlhandler.h
        mediaitem/gstcdplayback.h
        plugin.h
    )

add_library(plugin-gst SHARED ${SOURCES} ${HEADERS})
target_include_directories(plugin-gst PUBLIC ../../libthebeat/)

cntp_init_plugin(thebeat plugin-gst 20 gst)
cntp_translate(plugin-gst)

set_target_properties(plugin-gst PROPERTIES
        OUTPUT_NAME gstPlugin
        FRAMEWORK FALSE)

target_link_libraries(plugin-gst Qt6::Widgets Qt6::DBus PkgConfig::gstreamer libcontemporary libthebeat)
