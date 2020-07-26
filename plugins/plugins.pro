TEMPLATE = subdirs

unix:!macx {
    SUBDIRS += \
        PhononPlugin
}

DISTFILES += \
    plugins.pri
