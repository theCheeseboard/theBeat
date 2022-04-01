cmake_minimum_required(VERSION 3.21.0)

project(thebeat VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets DBus Sql PrintSupport)
find_package(libcontemporary REQUIRED)

set(SOURCES
        artistsalbumswidget.cpp
        common.cpp
        controlstrip.cpp
        currenttrackpopover.cpp
        library/libraryenumeratedirectoryjob.cpp
        library/libraryenumeratedirectoryjobwidget.cpp
        library/librarylistview.cpp
        library/librarymanager.cpp
        library/librarymodel.cpp
        libraryerrorpopover.cpp
        lyrics/abstractlyricformat.cpp
        lyrics/basiclrcfilelyricformat.cpp
        lyrics/lyricsdisplaywidget.cpp
        main.cpp
        mainwindow.cpp
        othersourceswidget.cpp
        playlistmodel.cpp
        pluginmanager.cpp
        print/printcontroller.cpp
        print/printsettings.cpp
        qtmultimedia/qtmultimediamediaitem.cpp
        qtmultimedia/qtmultimediaurlhandler.cpp
        settingsdialog.cpp
        tageditor/tageditor.cpp
        thememanager.cpp
        trackswidget.cpp
        userplaylistswidget.cpp
        visualisations/scopevisualisation.cpp

        artistsalbumswidget.ui
        controlstrip.ui
        currenttrackpopover.ui
        library/libraryenumeratedirectoryjobwidget.ui
        libraryerrorpopover.ui
        lyrics/lyricsdisplaywidget.ui
        mainwindow.ui
        othersourceswidget.ui
        print/printsettings.ui
        settingsdialog.ui
        tageditor/tageditor.ui
        trackswidget.ui
        userplaylistswidget.ui

        resources.qrc)

set(HEADERS
        artistsalbumswidget.h
        common.h
        controlstrip.h
        currenttrackpopover.h
        library/libraryenumeratedirectoryjob.h
        library/libraryenumeratedirectoryjobwidget.h
        library/librarylistview.h
        library/librarymanager.h
        library/librarymodel.h
        libraryerrorpopover.h
        lyrics/abstractlyricformat.h
        lyrics/basiclrcfilelyricformat.h
        lyrics/lyricsdisplaywidget.h
        mainwindow.h
        othersourceswidget.h
        playlistmodel.h
        pluginmanager.h
        print/printcontroller.h
        print/printsettings.h
        qtmultimedia/qtmultimediamediaitem.h
        qtmultimedia/qtmultimediaurlhandler.h
        settingsdialog.h
        tageditor/tageditor.h
        thememanager.h
        trackswidget.h
        userplaylistswidget.h
        visualisations/scopevisualisation.h)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(SOURCES ${SOURCES}
            platformintegration/winplatformintegration.cpp)

    set(HEADERS ${HEADERS}
            platformintegration/winplatformintegration.h)
ENDIF()

add_executable(thebeat ${SOURCES} ${HEADERS})

set_target_properties(thebeat PROPERTIES
        MACOSX_BUNDLE TRUE
        CNTP_DATA_SUBDIR thebeat)

cntp_init(thebeat 17)
cntp_translate(thebeat)

target_link_libraries(thebeat Qt6::Widgets Qt6::DBus Qt6::Sql Qt6::PrintSupport libcontemporary libthebeat)
target_include_directories(thebeat PUBLIC ../libthebeat/)
cntp_target_install_desktop_file_switch(thebeat com.vicr123.thebeat.desktop com.vicr123.thebeat_blueprint.desktop)

install(TARGETS thebeat
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION /Applications)