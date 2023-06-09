QT += widgets gui dbus thelib frisbee multimedia

TEMPLATE = lib
CONFIG += plugin

CONFIG += c++11

include(../plugins.pri)

unix {
    include($$THELIBS_BUILDTOOLS_PATH/gentranslations.pri)

    CONFIG += link_pkgconfig
    PKGCONFIG += taglib

    translations.files = translations/*.qm
    translations.path = $$THELIBS_INSTALL_PREFIX/share/thebeat/cdrdaoplugin/translations
    INSTALLS += translations
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

SOURCES += \
    abstractburnjob.cpp \
    burndevice.cpp \
    burnjob.cpp \
    burnjobmp3.cpp \
    burnjobwidget.cpp \
    burnpopover.cpp \
    drivewatcher.cpp \
    fullburnjob.cpp \
    plugin.cpp

HEADERS += \
    abstractburnjob.h \
    burndevice.h \
    burnjob.h \
    burnjobmp3.h \
    burnjobwidget.h \
    burnpopover.h \
    drivewatcher.h \
    fullburnjob.h \
    plugin.h

DISTFILES += CdrdaoPlugin.json \
    defaults.conf

FORMS += \
    burnjobwidget.ui \
    burnpopover.ui
