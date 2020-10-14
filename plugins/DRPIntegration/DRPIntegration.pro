QT += gui widgets

TEMPLATE = lib
CONFIG += plugin

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

unix:!macx {
    # Include the-libs build tools
    include(/usr/share/the-libs/pri/gentranslations.pri)
    QT += thelib

    DISCORD_PATH = /usr/lib/libdiscord-rpc.so
    DISCORD_STATIC_PATH = /usr/lib/libdiscord-rpc.a

    DISCORD_LIBS = -ldiscord-rpc
}

win32 {
    # Include the-libs build tools
    include(C:/Program Files/thelibs/pri/gentranslations.pri)
    INCLUDEPATH += "C:/Program Files/thelibs/include"
    LIBS += -L"C:/Program Files/thelibs/lib" -lthe-libs

    DISCORD_STATIC_PATH = "C:/Program Files (x86)/DiscordRPC/lib/discord-rpc.lib"

    DISCORD_LIBS = -L"C:/Program Files (x86)/DiscordRPC/lib/" -ldiscord-rpc -lAdvapi32
    DISCORD_INCLUDEPATH = "C:/Program Files (x86)/DiscordRPC/include/"
}

#Build Discord
DEFINES += BUILD_DISCORD
CONFIG += BUILD_DISCORD
LIBS += $$DISCORD_LIBS
INCLUDEPATH += $$DISCORD_INCLUDEPATH

exists($${DISCORD_STATIC_PATH}) {
    DEFINES += DISCORD_STATIC
}

SOURCES += \
    plugin.cpp

HEADERS += \
    plugin.h

DISTFILES += DRPIntegration.json \
    defaults.conf

include(../plugins.pri)
