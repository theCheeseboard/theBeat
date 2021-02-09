TEMPLATE = subdirs

unix:!macx {
    SUBDIRS += \
        LinuxIntegration \
        CdrdaoPlugin
}

win32 {
    SUBDIRS += \
        WinLibCDPlugin
}

macx {
    SUBDIRS += \
        MacIntegration \
        AvFoundationPlugin
}


#Determine whether to build Discord
no-discord {
    #Don't build Discord
} else {
    unix:!macx {
        DISCORD_PATH = /usr/lib/libdiscord-rpc.so
        DISCORD_STATIC_PATH = /usr/lib/libdiscord-rpc.a

        DISCORD_LIBS = -ldiscord-rpc
    }

    win32 {
        DISCORD_STATIC_PATH = "C:/Program Files (x86)/DiscordRPC/lib/discord-rpc.lib"

        DISCORD_LIBS = -L"C:/Program Files (x86)/DiscordRPC/lib/" -ldiscord-rpc -lAdvapi32
        DISCORD_INCLUDEPATH = "C:/Program Files (x86)/DiscordRPC/include/"
    }

    exists($${DISCORD_STATIC_PATH}) || exists($${DISCORD_PATH}) || discord {
        #Build Discord
        message(Building with Discord RPC support)
        SUBDIRS += DRPIntegration
    } else {
        message(Discord RPC library not found)
    }
}

qtHaveModule(phonon4qt5) {
    message("Building with Phonon support")
    SUBDIRS += PhononPlugin
}

DISTFILES += \
    plugins.pri

