TEMPLATE = subdirs

unix:!macx {
    SUBDIRS += \
        PhononPlugin \
        LinuxIntegration \
        CdrdaoPlugin
}

win32 {
    SUBDIRS += \
        WinLibCDPlugin
}

DISTFILES += \
    plugins.pri

