QT += gui widgets

TEMPLATE = lib
CONFIG += plugin

CONFIG += c++11

unix {
    CONFIG += link_pkgconfig

    packagesExist(x11) {
        message("Building with X11 support");
        PKGCONFIG += x11
        DEFINES += HAVE_X11
        QT += x11extras
    } else {
        message("X11 not found on this system");
    }
}

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Include the-libs build tools
include(/usr/share/the-libs/pri/gentranslations.pri)

SOURCES += \
    nativeevents.cpp \
    plugin.cpp

HEADERS += \
    nativeevents.h \
    plugin.h

DISTFILES += LinuxIntegration.json \
    defaults.conf

include(../plugins.pri)
