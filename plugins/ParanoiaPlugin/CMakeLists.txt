project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets DBus)
find_package(libcontemporary)

find_package(PkgConfig REQUIRED)
find_package(libthefrisbee)
pkg_check_modules(cdio IMPORTED_TARGET libcdio++)
pkg_check_modules(MusicBrainz IMPORTED_TARGET libmusicbrainz5)
pkg_check_modules(cdio-paranoia IMPORTED_TARGET libcdio_paranoia)

IF(NOT ${cdio-paranoia_FOUND})
return()
ENDIF()

set(SOURCES
        plugin.cpp
    )

set(HEADERS
        plugin.h
    )

add_library(plugin-paranoia SHARED ${SOURCES} ${HEADERS})
target_include_directories(plugin-paranoia PUBLIC ../../libthebeat/)

cntp_init_plugin(thebeat plugin-paranoia 20 paranoia)
cntp_translate(plugin-paranoia)

set_target_properties(plugin-paranoia PROPERTIES
        OUTPUT_NAME paranoiaPlugin
        FRAMEWORK FALSE)

qt_add_qml_module(plugin-paranoia
    URI com.vicr123.thebeat.plugin.paranoia
    VERSION 1.0
    QML_FILES
        ParanoiaCdPane.qml
    NO_CACHEGEN
    SOURCES paranoiacdcontroller.h paranoiacdcontroller.cpp
    SOURCES paranoiatrackinfo.h paranoiatrackinfo.cpp
    QML_FILES ParanoiaPlugin.json
    SOURCES paranoiacdpluginmediasource.h paranoiacdpluginmediasource.cpp
    SOURCES paranoiaplayer.h paranoiaplayer.cpp
    SOURCES paranoiamediaitem.h paranoiamediaitem.cpp
)

target_link_libraries(plugin-paranoia PRIVATE Qt::Widgets Qt::DBus PkgConfig::cdio PkgConfig::cdio-paranoia libcontemporary libthebeat)

IF(${libthefrisbee_FOUND})
    target_sources(plugin-paranoia PRIVATE cdmonitor.cpp cdmonitor.h)
    target_link_libraries(plugin-paranoia PRIVATE libthefrisbee)
    target_compile_definitions(plugin-paranoia PUBLIC HAVE_THEFRISBEE)
ENDIF()


IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_sources(plugin-paranoia PRIVATE wincdmonitor.h wincdmonitor.cpp)
    target_link_libraries(plugin-paranoia PRIVATE comsuppwd)
ENDIF()

IF(${MusicBrainz_FOUND})
    target_link_libraries(plugin-paranoia PRIVATE PkgConfig::MusicBrainz)
    target_compile_definitions(plugin-paranoia PUBLIC HAVE_MUSICBRAINZ)
ENDIF()
