project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Concurrent)
find_package(libcontemporary REQUIRED)

set(CNTP_TARGET_ARCH ${CMAKE_SYSTEM_PROCESSOR} CACHE STRING "Architecture to build for")

include(ExternalProject)

function(translate_arch arch)
    if(${arch} STREQUAL "AMD64")
        set(RESULT_ARCH "x64" PARENT_SCOPE)
    elseif(${arch} STREQUAL "arm64")
        set(RESULT_ARCH "arm64" PARENT_SCOPE)
    else()
        set(RESULT_ARCH ${arch} PARENT_SCOPE)
    endif()
endfunction()

if(";${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION};$ENV{UCRTVersion};$ENV{WindowsSDKVersion};" MATCHES [=[;(10\.[0-9.]+)[;\]]=])
    get_filename_component(WINDOWS_KITS_BASE_DIR
        "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE)

    set(WINDOWS_SDK_VERSION ${CMAKE_MATCH_1})
    translate_arch(CMAKE_HOST_SYSTEM_PROCESSOR)
    set(CPPWINRT_PATH "${WINDOWS_KITS_BASE_DIR}/bin/${WINDOWS_SDK_VERSION}/${RESULT_ARCH}/cppwinrt.exe")
endif()

translate_arch(${CNTP_TARGET_ARCH})

ExternalProject_Add(CDLib
  GIT_REPOSITORY    https://github.com/reflectronic/cdlib.git
  GIT_TAG           21f9f6abd03291a56a6b5875f6afc6c67c1d5795
  PREFIX            ${CMAKE_BINARY_DIR}/cdlib
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     msbuild /m /v:m /r /p:OutDir=<BINARY_DIR>/ /p:RestorePackagesConfig=true /p:Configuration=$<CONFIG> /p:Platform=${RESULT_ARCH} <SOURCE_DIR>/CDLib.sln
  INSTALL_COMMAND   ${CPPWINRT_PATH} -out <INSTALL_DIR>/Include/ -in <BINARY_DIR>/CDLib.winmd -ref ${WINDOWS_SDK_VERSION}
)

ExternalProject_Get_Property(CDLib INSTALL_DIR)

set(SOURCES
        burn/cdtextcrcgenerator.cpp
        burn/cdtextgenerator.cpp
        burn/winburndaoimage.cpp
        burn/winburnjob.cpp
        burn/winburnjobwidget.cpp
        burn/winburnmanager.cpp
        burn/winburnpopover.cpp
        burn/winburnprovider.cpp
        cdplayback/audiocdplayerthread.cpp
        cdplayback/cdchecker.cpp
        cdplayback/diskwatcher.cpp
        cdplayback/trackinfo.cpp
        cdplayback/wincdmediaitem.cpp
        plugin.cpp
        smtcintegration.cpp

        burn/winburnjobwidget.ui
        burn/winburnpopover.ui
        cdplayback/cdchecker.ui)

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
        cdplayback/audiocdplayerthread.h
        cdplayback/cdchecker.h
        cdplayback/diskwatcher.h
        cdplayback/trackinfo.h
        cdplayback/wincdmediaitem.h
        plugin.h
        smtcintegration.h)

add_library(plugin-winintegration SHARED ${SOURCES} ${HEADERS})
target_include_directories(plugin-winintegration PUBLIC ../../libthebeat/ ${INSTALL_DIR}/Include)
add_dependencies(plugin-winintegration CDLib)

cntp_init_plugin(thebeat plugin-winintegration 20 linuxintegration)
cntp_translate(plugin-winintegration)

set_target_properties(plugin-winintegration PROPERTIES
        OUTPUT_NAME winIntegration
        FRAMEWORK FALSE)

target_link_libraries(plugin-winintegration Qt::Widgets Qt::DBus Qt::Concurrent libcontemporary libthebeat User32 windowsapp comsuppwd shlwapi)
