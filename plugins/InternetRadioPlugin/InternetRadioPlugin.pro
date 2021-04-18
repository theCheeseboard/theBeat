QT += gui widgets network

TEMPLATE = lib
CONFIG += plugin

CONFIG += c++11

include(../plugins.pri)

unix {
    include($$THELIBS_BUILDTOOLS_PATH/gentranslations.pri)

    QT += thelib

    translations.files = translations/*.qm
    translations.path = $$THELIBS_INSTALL_PREFIX/share/thebeat/internetradioplugin/translations
    INSTALLS += translations
}

win32 {
    # Include the-libs build tools
    include(C:/Program Files/thelibs/pri/gentranslations.pri)

    INCLUDEPATH += "C:/Program Files/thelibs/include"
    LIBS += -L"C:/Program Files/thelibs/lib" -lthe-libs
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

FORMS += \
    favouritestationswidget.ui \
    radiopane.ui \
    stationsearchwidget.ui \
    stationwidget.ui
