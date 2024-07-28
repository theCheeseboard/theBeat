cmake_minimum_required(VERSION 3.21.0)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)

project(thebeat VERSION 1.0.0 LANGUAGES CXX)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    link_directories(/usr/local/lib)
ENDIF()

find_package(QT REQUIRED NAMES Qt6)
find_package(Qt${QT_VERSION_MAJOR} 6.4 REQUIRED COMPONENTS Quick QuickControls2)

find_package(QT REQUIRED NAMES Qt6)

qt_policy(
    SET QTP0001 NEW
)

qt_standard_project_setup()

add_subdirectory(libthebeat)
add_subdirectory(application)
add_subdirectory(plugins)
