CONFIG += plugin

INCLUDEPATH += $$PWD/../libthebeat
DEPENDPATH += $$PWD/../libthebeat

# Include the-libs build tools
equals(THELIBS_BUILDTOOLS_PATH, "") {
    THELIBS_BUILDTOOLS_PATH = $$[QT_INSTALL_PREFIX]/share/the-libs/pri
}
include($$THELIBS_BUILDTOOLS_PATH/varset.pri)

unix:!macx: {
    LIBS += -L$$OUT_PWD/../../libthebeat/ -lthebeat

    target.path = $$THELIBS_INSTALL_LIB/thebeat/plugins
    INSTALLS += target translations

    QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$quote($$_PRO_FILE_PWD_/translations) $$shell_quote($$OUT_PWD) && \
        $$QMAKE_COPY $$quote($$_PRO_FILE_PWD_/defaults.conf) $$shell_quote($$OUT_PWD)
}

win32 {
    CONFIG(release, debug|release) {
        LIBS += -L$$OUT_PWD/../../libthebeat/release/ -lthebeat
    } else {
        CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../libthebeat/debug/ -lthebeat
    }
}

macx {
    LIBS += -L$$OUT_PWD/../../libthebeat/ -lthebeat
}
