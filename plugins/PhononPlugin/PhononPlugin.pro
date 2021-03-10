QT += gui widgets phonon4qt5 dbus thelib network multimedia

TEMPLATE = lib
CONFIG += plugin

CONFIG += c++11

# Include the-libs build tools
isEmpty(THELIBS_BUILDTOOLS_PATH) {
    THELIBS_BUILDTOOLS_PATH = $$[QT_INSTALL_PREFIX]/share/the-libs/pri
}
include($$THELIBS_BUILDTOOLS_PATH/buildmaster.pri)

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += taglib

    translations.files = translations/*.qm
    translations.path = $$THELIBS_INSTALL_PREFIX/share/thebeat/phononplugin/translations
    INSTALLS += translations

    packagesExist(libmusicbrainz5) {
        message("Building with libmusicbrainz5 support")
        PKGCONFIG += libmusicbrainz5
        DEFINES += HAVE_MUSICBRAINZ

        SOURCES += musicbrainzreleasemodel.cpp
        HEADERS += musicbrainzreleasemodel.h
    } else {
        message("libmusicbrainz5 not found on this system");
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

SOURCES += \
    cdchecker.cpp \
    importcdjob.cpp \
    importcdjobwidget.cpp \
    importcdpopover.cpp \
    phononcdmediaitem.cpp \
    plugin.cpp \
    trackinfo.cpp \
    udiskswatcher.cpp

HEADERS += \
    cdchecker.h \
    importcdjob.h \
    importcdjobwidget.h \
    importcdpopover.h \
    phononcdmediaitem.h \
    plugin.h \
    trackinfo.h \
    udiskswatcher.h

DISTFILES += PhononPlugin.json

include(../plugins.pri)

FORMS += \
    cdchecker.ui \
    importcdjobwidget.ui \
    importcdpopover.ui
