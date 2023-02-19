project(lib VERSION 1.0.0 LANGUAGES CXX)

set(TAGLIB_ROOT "C:/Program Files (x86)/taglib" CACHE PATH "Path to taglib on Windows")

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia)
find_package(libcontemporary REQUIRED)
cntp_find_pkgconfig(tag taglib REQUIRED
        WIN_FALLBACK_DLL "${TAGLIB_ROOT}/bin/tag.dll"
        WIN_FALLBACK_IMPLIB "${TAGLIB_ROOT}/lib/tag.lib"
        WIN_FALLBACK_INCLUDE "${TAGLIB_ROOT}/include")

set(SOURCES
        burnbackend.cpp
        burnmanager.cpp
        controlstripmanager.cpp
        helpers.cpp
        mediaitem.cpp
        playlist.cpp
        pluginmediasource.cpp
        sourcemanager.cpp
        statemanager.cpp
        urlhandler.cpp
        urlmanager.cpp
        visualisationengine.cpp
        visualisationmanager.cpp
    )

set(HEADERS
        abstractlibrarybrowser.h
        burnbackend.h
        burnmanager.h
        controlstripmanager.h
        helpers.h
        libthebeat_global.h
        mediaitem.h
        playlist.h
        thebeatplugininterface.h
        pluginmediasource.h
        sourcemanager.h
        statemanager.h
        urlhandler.h
        urlmanager.h
        visualisationengine.h
        visualisationmanager.h
    )

add_library(libthebeat SHARED ${SOURCES} ${HEADERS})
cntp_init(libthebeat 20)
set_target_properties(libthebeat PROPERTIES
        OUTPUT_NAME thebeat
        FRAMEWORK TRUE
        MACOSX_FRAMEWORK_IDENTIFIER com.vicr123.libthebeat
        VERSION 1.0.0
        PUBLIC_HEADER "${HEADERS}")

target_link_libraries(libthebeat Qt::Widgets Qt::DBus Qt::Multimedia libcontemporary PkgConfig::tag)
target_compile_definitions(libthebeat PRIVATE LIBTHEBEAT_LIBRARY)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKECONFIG_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/libthebeat.framework/Resources/CMake)
    set(HEADER_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
    set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/libthebeat.framework/Headers)
    set(LIBRARY_INSTALL_DIR ../)
ELSE()
    set(CMAKECONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/libthebeat)
    set(HEADER_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/libthebeat)
    set(INCLUDE_INSTALL_DIR ${HEADER_INSTALL_DIR})
    set(LIBRARY_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
ENDIF()

configure_package_config_file(libthebeatConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/libthebeatConfig.cmake
        INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
        PATH_VARS HEADER_INSTALL_DIR LIBRARY_INSTALL_DIR)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libthebeatConfig.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})
install(EXPORT libthebeatTargets
        FILE libthebeatTargets.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})

install(TARGETS libthebeat
        EXPORT libthebeatTargets
        PUBLIC_HEADER DESTINATION ${HEADER_INSTALL_DIR}
        INCLUDES DESTINATION ${INCLUDE_INSTALL_DIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR})

cntp_mark_support_library_directory()
