cntp_add_qml_module(library thebeat
    URI com.vicr123.thebeat.library
    VERSION 1.0
    SOURCES
        librarymodel.h librarymodel.cpp
)

target_link_libraries(thebeat-qmlmodule-library PRIVATE libthebeat Qt::Sql)
target_include_directories(thebeat-qmlmodule-library PRIVATE ../../libthebeat)
