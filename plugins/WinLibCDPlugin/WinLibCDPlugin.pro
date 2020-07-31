QT += gui widgets

TEMPLATE = lib
CONFIG += plugin

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Link with CDLib
LIBS += -L$$PWD/cdlib/x64/Release -lCDLib -lwindowsapp
INCLUDEPATH += $$PWD/cdlib/include/

SOURCES += \
    plugin.cpp

HEADERS += \
    plugin.h

DISTFILES += WinLibCDPlugin.json

# Include the-libs build tools
include(C:/Program Files/thelibs/pri/buildmaster.pri)

include(../plugins.pri)
