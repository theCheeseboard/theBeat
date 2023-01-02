cmake_minimum_required(VERSION 3.21.0)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)

project(thebeat VERSION 1.0.0 LANGUAGES CXX)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    link_directories(/usr/local/lib)
ENDIF()

add_subdirectory(libthebeat)
add_subdirectory(application)
add_subdirectory(plugins)
