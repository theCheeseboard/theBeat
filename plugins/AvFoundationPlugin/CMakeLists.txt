cmake_minimum_required(VERSION 3.4.0)

project(lib VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets DBus)
find_package(libcontemporary)
find_library(AVFOUNDATION_FRAMEWORK AVFoundation REQUIRED)
find_library(COREMEDIA_FRAMEWORK CoreMedia REQUIRED)
find_library(AVKIT_FRAMEWORK AvKit REQUIRED)

set(SOURCES
        airplaymanager.mm
        avfoundationmediaitem.mm
        avfoundationurlhandler.cpp
        avplayerinstance.mm
        plugin.cpp)

set(HEADERS
        airplaymanager.h
        avfoundationmediaitem.h
        avfoundationurlhandler.h
        avplayerinstance.h
        plugin.h)

add_library(plugin-avfoundation SHARED ${SOURCES} ${HEADERS})
target_include_directories(plugin-avfoundation PUBLIC ../../libthebeat/)

cntp_init_plugin(thebeat plugin-avfoundation 17 avfoundation)
cntp_translate(plugin-avfoundation)

set_target_properties(plugin-avfoundation PROPERTIES
        OUTPUT_NAME avFoundationPlugin
        FRAMEWORK FALSE)

target_link_libraries(plugin-avfoundation Qt6::Widgets Qt6::Multimedia libcontemporary libthebeat ${AVFOUNDATION_FRAMEWORK} ${COREMEDIA_FRAMEWORK} ${AVKIT_FRAMEWORK})