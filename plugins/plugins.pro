TEMPLATE = subdirs

unix:!macx {
    SUBDIRS += \
        PhononPlugin \
        LinuxIntegration
}

win32 {
    SUBDIRS += \
        WinLibCDPlugin
}

DISTFILES += \
    plugins.pri

