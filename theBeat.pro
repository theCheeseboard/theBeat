#-------------------------------------------------
#
# Project created by QtCreator 2017-06-18T15:43:15
#
#-------------------------------------------------

QT       += core gui phonon4qt5 dbus x11extras thelib svg network
LIBS     += -ltag
CONFIG   += c++14

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += x11 libmusicbrainz5
}

#Determine whether to build Discord
no-discord {
    #Don't build Discord
} else {
    exists(/usr/lib/libdiscord-rpc.so) || discord {
        #Build Discord
        DEFINES += BUILD_DISCORD
        CONFIG += BUILD_DISCORD
    }
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = thebeat
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    visualisationframe.cpp \
    playlistmodel.cpp \
    dbusadaptors.cpp \
    aboutwindow.cpp \
    librarymodel.cpp \
    playlistlistwidget.cpp \
    nativeeventfilter.cpp \
    librarymanagewidget.cpp \
    tagcache.cpp \
    secondaryinformationlistdelegate.cpp \
    cdmodel.cpp

BUILD_DISCORD {
    SOURCES += discordintegration.cpp
}

HEADERS += \
        mainwindow.h \
    visualisationframe.h \
    playlistmodel.h \
    dbusadaptors.h \
    aboutwindow.h \
    librarymodel.h \
    playlistlistwidget.h \
    nativeeventfilter.h \
    librarymanagewidget.h \
    tagcache.h \
    secondaryinformationlistdelegate.h \
    cdmodel.h

BUILD_DISCORD {
    HEADERS += discordintegration.h
}

FORMS += \
        mainwindow.ui \
    aboutwindow.ui \
    librarymanagewidget.ui

RESOURCES += \
    resources.qrc

DISTFILES += \
    thebeat.desktop \
    thebeat.svg

TRANSLATIONS += translations/vi_VN.ts \
    translations/da_DK.ts \
    translations/es_ES.ts \
    translations/lt_LT.ts \
    translations/nl_NL.ts \
    translations/pl_PL.ts \
    translations/pt_BR.ts \
    translations/ru_RU.ts \
    translations/sv_SE.ts \
    translations/en_AU.ts \
    translations/en_US.ts \
    translations/en_GB.ts \
    translations/en_NZ.ts \
    translations/de_DE.ts \
    translations/id_ID.ts \
    translations/au_AU.ts \
    translations/it_IT.ts \
    translations/nb_NO.ts \
    translations/no_NO.ts \
    translations/ro_RO.ts \
    translations/cy_GB.ts \
    translations/fr_FR.ts

qtPrepareTool(LUPDATE, lupdate)
genlang.commands = "$$LUPDATE -no-obsolete -source-language en_US $$_PRO_FILE_"

qtPrepareTool(LRELEASE, lrelease)
rellang.commands = "$$LRELEASE -removeidentical $$_PRO_FILE_"
QMAKE_EXTRA_TARGETS = genlang rellang
PRE_TARGETDEPS = genlang rellang

# Turn off stripping as this causes the install to fail :(
QMAKE_STRIP = echo

unix {
    target.path = /usr/bin

    appentry.path = /usr/share/applications
    appentry.files = thebeat.desktop

    icon.path = /usr/share/icons
    icon.files = thebeat.svg

    translations.path = /usr/share/thebeat/translations
    translations.files = translations/*

    INSTALLS += target appentry icon
}
