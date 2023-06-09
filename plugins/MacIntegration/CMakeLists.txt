project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets DBus)
find_package(libcontemporary)
find_package(PkgConfig REQUIRED)
find_library(AVFOUNDATION_FRAMEWORK AVFoundation REQUIRED)
find_library(DISCRECORDING_FRAMEWORK DiscRecording REQUIRED)
find_library(DISCRECORDINGUI_FRAMEWORK DiscRecordingUI REQUIRED)
find_library(MEDIAPLAYER_FRAMEWORK MediaPlayer REQUIRED)

pkg_check_modules(MusicBrainz IMPORTED_TARGET libmusicbrainz5)

set(SOURCES
        ../PhononPlugin/musicbrainzreleasemodel.cpp
        burn/macburnjob.mm
        burn/macburnjobwidget.cpp
        burn/macburnprovider-objc.mm
        burn/macburnprovider.cpp
        burn/macburntrack.mm
        cd/cdchecker-objc.mm
        cd/cdchecker.cpp
        cd/diskwatcher.cpp
        cd/maccdmediaitem.cpp
        cd/trackinfo.cpp
        mainwindowtouchbar-objc.mm
        mainwindowtouchbar.cpp
        nowplayingintegration.mm
        plugin.cpp

        burn/macburnjobwidget.ui
        cd/cdchecker.ui
    )

set(HEADERS
        ../PhononPlugin/musicbrainzreleasemodel.h
        burn/macburnjob.h
        burn/macburnjobwidget.h
        burn/macburnprovider.h
        burn/macburntrack.h
        cd/cdchecker.h
        cd/cdchecker_p.h
        cd/diskwatcher.h
        cd/maccdmediaitem.h
        cd/trackinfo.h
        mainwindowtouchbar.h
        mainwindowtouchbar_p.h
        nowplayingintegration.h
        plugin.h)

add_library(plugin-macintegration SHARED ${SOURCES} ${HEADERS})
target_include_directories(plugin-macintegration PUBLIC ../../libthebeat/)

cntp_init_plugin(thebeat plugin-macintegration 20 macintegration)
cntp_translate(plugin-macintegration)

set_target_properties(plugin-macintegration PROPERTIES
        OUTPUT_NAME macIntegrationPlugin
        FRAMEWORK FALSE)

target_link_libraries(plugin-macintegration Qt::Widgets Qt::Multimedia libcontemporary libthebeat ${AVFOUNDATION_FRAMEWORK} ${DISCRECORDING_FRAMEWORK} ${DISCRECORDINGUI_FRAMEWORK} ${MEDIAPLAYER_FRAMEWORK})

IF(${MusicBrainz_FOUND})
    target_link_libraries(plugin-macintegration PkgConfig::MusicBrainz)
    target_compile_definitions(plugin-macintegration PUBLIC HAVE_MUSICBRAINZ)
ENDIF()
