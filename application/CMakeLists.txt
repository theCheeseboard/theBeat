project(thebeat VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets DBus Sql PrintSupport)
find_package(libcontemporary REQUIRED)

set(SOURCES
        artistsalbumswidget.cpp artistsalbumswidget.ui
        common.cpp
        controlstrip.cpp controlstrip.ui
        currenttrackpopover.cpp currenttrackpopover.ui
        library/libraryenumeratedirectoryjob.cpp
        library/libraryenumeratedirectoryjobwidget.cpp library/libraryenumeratedirectoryjobwidget.ui
        library/librarylistview.cpp
        library/librarymanager.cpp
        library/librarymodel.cpp
        libraryerrorpopover.cpp libraryerrorpopover.ui
        lyrics/abstractlyricformat.cpp
        lyrics/basiclrcfilelyricformat.cpp
        lyrics/lyricsdisplaywidget.cpp lyrics/lyricsdisplaywidget.ui
        main.cpp
        mainwindow.cpp mainwindow.ui
        othersourceswidget.cpp othersourceswidget.ui
        playlistmodel.cpp
        print/printcontroller.cpp
        print/printsettings.cpp print/printsettings.ui
        qtmultimedia/qtmultimediamediaitem.cpp
        qtmultimedia/qtmultimediaurlhandler.cpp
        tageditor/tageditor.cpp tageditor/tageditor.ui
        thememanager.cpp
        trackswidget.cpp trackswidget.ui
        userplaylistswidget.cpp userplaylistswidget.ui
        visualisations/scopevisualisation.cpp
        settingspanes/libraryresetsettingspane.cpp settingspanes/libraryresetsettingspane.ui
        settingspanes/titlebarsettingspane.cpp settingspanes/titlebarsettingspane.ui
        settingspanes/colourssettingspane.cpp settingspanes/colourssettingspane.ui
        settingspanes/notificationssettingspane.cpp settingspanes/notificationssettingspane.ui
        resetlibrarypopover.cpp resetlibrarypopover.ui

        commandpalette/artistsalbumscommandpalettescope.cpp
        commandpalette/trackscommandpalettescope.cpp

        resources.qrc

        Info.plist.in
    )

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
        print/printcontroller.h
        print/printsettings.h
        qtmultimedia/qtmultimediamediaitem.h
        qtmultimedia/qtmultimediaurlhandler.h
        tageditor/tageditor.h
        thememanager.h
        trackswidget.h
        userplaylistswidget.h
        visualisations/scopevisualisation.h
        settingspanes/libraryresetsettingspane.h
        settingspanes/titlebarsettingspane.h
        settingspanes/colourssettingspane.h
        settingspanes/notificationssettingspane.h
        commandpalette/artistsalbumscommandpalettescope.h
        commandpalette/trackscommandpalettescope.h
        resetlibrarypopover.h
    )

# Find Apple lproj files
set(LPROJ_SOURCES)
file(GLOB LPROJ_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/translations/apple-lproj/*.lproj)
FOREACH(DIR ${LPROJ_DIRS})
    cmake_path(GET DIR FILENAME DIR_NAME)
    file(GLOB LPROJ_FILES ${DIR}/*.strings)
    FOREACH(FILE ${LPROJ_FILES})
        list(APPEND LPROJ_SOURCES ${FILE})
        set_source_files_properties(${FILE} PROPERTIES
                MACOSX_PACKAGE_LOCATION Resources/${DIR_NAME})
    ENDFOREACH()
ENDFOREACH()
set(SOURCES ${SOURCES} ${LPROJ_SOURCES})

add_executable(thebeat ${SOURCES} ${HEADERS})

set_target_properties(thebeat PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
        MACOSX_BUNDLE_GUI_IDENTIFIER com.vicr123.thebeat
        WIN32_EXECUTABLE TRUE
        CNTP_DATA_SUBDIR thebeat
        TRANSLATION_SOURCES "${SOURCES};${HEADERS}"
    )

cntp_init(thebeat 20)
cntp_translate(thebeat)
cntp_target_name(thebeat "theBeat")
cntp_app_icon(thebeat
    BASEICON icons/baseicon.svg
    BASEICON_MAC icons/baseicon-mac.svg)
cntp_dmg(thebeat app-dmg-background.svg)
cntp_target_desktop_file(thebeat
    DESKTOP_FILE com.vicr123.thebeat.desktop
    BLUEPRINT_DESKTOP_FILE com.vicr123.thebeat_blueprint.desktop)
cntp_target_appstream_metainfo_file(thebeat
    METAINFO_FILE com.vicr123.thebeat.metainfo.xml
    BLUEPRINT_METAINFO_FILE com.vicr123.thebeat_blueprint.metainfo.xml)
cntp_defaults_file(thebeat
    DEFAULTS_FILE thebeat.conf)

target_link_libraries(thebeat Qt::Widgets Qt::DBus Qt::Sql Qt::PrintSupport libcontemporary libthebeat)
target_include_directories(thebeat PUBLIC ../libthebeat/)

install(TARGETS thebeat
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION /Applications)
