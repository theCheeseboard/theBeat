project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets DBus)
find_package(libcontemporary)

set(SOURCES
    plugin.cpp
    podcastmanager.cpp
    podcast.cpp
    podcastitem.cpp
    podcastcommon.cpp
    podcastmediaitem.cpp

    widgets/podcastpane.cpp widgets/podcastpane.ui
    widgets/subscribedpodcastswidget.cpp widgets/subscribedpodcastswidget.ui
    widgets/podcastlistingwidget.cpp widgets/podcastlistingwidget.ui
    widgets/podcastitemwidget.cpp widgets/podcastitemwidget.ui
)

set(HEADERS
    plugin.h
    podcastmanager.h
    podcast.h
    podcastitem.h
    podcastcommon.h
    podcastmediaitem.h

    widgets/podcastpane.h
    widgets/subscribedpodcastswidget.h
    widgets/podcastlistingwidget.h
    widgets/podcastitemwidget.h
)

add_library(plugin-podcast SHARED ${SOURCES} ${HEADERS})
target_include_directories(plugin-podcast PUBLIC ../../libthebeat/)

cntp_init_plugin(thebeat plugin-podcast 20 podcast)
cntp_translate(plugin-podcast)

set_target_properties(plugin-podcast PROPERTIES
        OUTPUT_NAME podcastPlugin
        FRAMEWORK FALSE)

target_link_libraries(plugin-podcast Qt::Widgets Qt::DBus libcontemporary libthebeat)
