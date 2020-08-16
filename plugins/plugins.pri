CONFIG += plugin

INCLUDEPATH += $$PWD/../libthebeat
DEPENDPATH += $$PWD/../libthebeat

unix:!macx: {
    LIBS += -L$$OUT_PWD/../../libthebeat/ -lthebeat

    CI = $$(CI)
    if (isEmpty(CI)) {
        target.path = $$[QT_INSTALL_LIBS]/thebeat/plugins
    } else {
        target.path = /usr/lib/thebeat/plugins
    }
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
