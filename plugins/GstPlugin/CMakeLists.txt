project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets DBus)
find_package(libcontemporary)
find_package(libthefrisbee)
pkg_check_modules(gstreamer IMPORTED_TARGET REQUIRED gstreamer-1.0)
pkg_check_modules(cdio IMPORTED_TARGET libcdio++)
pkg_check_modules(MusicBrainz IMPORTED_TARGET libmusicbrainz5)

set(SOURCES
        mediaitem/gstmediaitem.cpp
        mediaitem/gsturlhandler.cpp
        mediaitem/gstcdplayback.cpp
        gstcdcontroller.cpp
        cdmonitor.cpp
        plugin.cpp
    )

set(HEADERS
        mediaitem/gstmediaitem.h
        mediaitem/gsturlhandler.h
        mediaitem/gstcdplayback.h
        gstcdcontroller.h
        cdmonitor.h
        plugin.h
    )

add_library(plugin-gst SHARED ${SOURCES} ${HEADERS}
    mediaitem/gstcdpluginmediasource.h mediaitem/gstcdpluginmediasource.cpp)
target_include_directories(plugin-gst PUBLIC ../../libthebeat/)

cntp_init_plugin(thebeat plugin-gst 20 gst)
cntp_translate(plugin-gst)

set_target_properties(plugin-gst PROPERTIES
        OUTPUT_NAME gstPlugin
        FRAMEWORK FALSE)

qt_add_qml_module(plugin-gst
    URI com.vicr123.thebeat.plugin.gst
    VERSION 1.0
    QML_FILES
        GstCdPane.qml
    NO_CACHEGEN
    SOURCES gsttrackinfo.h gsttrackinfo.cpp
)

target_link_libraries(plugin-gst PRIVATE Qt::Widgets Qt::DBus PkgConfig::gstreamer libcontemporary libthefrisbee libthebeat)

IF(${cdio_FOUND})
    target_link_libraries(plugin-gst PRIVATE PkgConfig::cdio)
    target_compile_definitions(plugin-gst PUBLIC HAVE_CDIO)
ENDIF()

IF(${MusicBrainz_FOUND})
    target_link_libraries(plugin-gst PRIVATE PkgConfig::MusicBrainz)
    target_compile_definitions(plugin-gst PUBLIC HAVE_MUSICBRAINZ)
ENDIF()
