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

qtHaveModule(phonon4qt5) {
    message("Building with Phonon support")
    SUBDIRS += PhononPlugin
}

DISTFILES += \
    plugins.pri

