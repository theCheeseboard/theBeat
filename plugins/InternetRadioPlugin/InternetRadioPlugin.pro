QT += gui widgets thelib network

TEMPLATE = lib
CONFIG += plugin

CONFIG += c++11

include(../plugins.pri)

unix {
    translations.files = translations/*.qm
    translations.path = $$THELIBS_INSTALL_PREFIX/share/thebeat/internetradioplugin/translations
    INSTALLS += translations
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    favouritestationswidget.cpp \
    plugin.cpp \
    radioinfoclient.cpp \
    radiopane.cpp \
    stationsearchwidget.cpp \
    stationwidget.cpp

HEADERS += \
    favouritestationswidget.h \
    plugin.h \
    radioinfoclient.h \
    radiopane.h \
    stationsearchwidget.h \
    stationwidget.h

DISTFILES += InternetRadioPlugin.json \
    defaults.conf

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

FORMS += \
    favouritestationswidget.ui \
    radiopane.ui \
    stationsearchwidget.ui \
    stationwidget.ui
