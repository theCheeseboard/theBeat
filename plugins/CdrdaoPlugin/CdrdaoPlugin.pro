QT += widgets gui dbus thelib

TEMPLATE = lib
CONFIG += plugin

CONFIG += c++11

# Include the-libs build tools
equals(THELIBS_BUILDTOOLS_PATH, "") {
    THELIBS_BUILDTOOLS_PATH = $$[QT_INSTALL_PREFIX]/share/the-libs/pri
}
include($$THELIBS_BUILDTOOLS_PATH/buildmaster.pri)

unix {
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
    burndevice.cpp \
    burnjob.cpp \
    burnjobmp3.cpp \
    burnjobwidget.cpp \
    burnpopover.cpp \
    plugin.cpp

HEADERS += \
    burndevice.h \
    burnjob.h \
    burnjobmp3.h \
    burnjobwidget.h \
    burnpopover.h \
    plugin.h

DISTFILES += CdrdaoPlugin.json \
    defaults.conf

include(../plugins.pri)

FORMS += \
    burnjobwidget.ui \
    burnpopover.ui
