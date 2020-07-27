TEMPLATE = subdirs

unix:!macx {
    SUBDIRS += \
        PhononPlugin \
        LinuxIntegration
}

DISTFILES += \
    plugins.pri

