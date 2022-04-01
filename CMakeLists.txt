cmake_minimum_required(VERSION 3.1.0)

project(thebeat VERSION 1.0.0 LANGUAGES CXX)

add_subdirectory(libthebeat)
add_subdirectory(application)
add_subdirectory(plugins)