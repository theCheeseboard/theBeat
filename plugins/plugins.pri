CONFIG += plugin

INCLUDEPATH += $$PWD/../libthebeat
DEPENDPATH += $$PWD/../libthebeat

unix:!macx: {
    LIBS += -L$$OUT_PWD/../../libthebeat/ -lthebeat

    target.path = $$[QT_INSTALL_LIBS]/thebeat/plugins
    INSTALLS += target
}

QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$quote($$_PRO_FILE_PWD_/translations) $$shell_quote($$OUT_PWD) && \
    $$QMAKE_COPY $$quote($$_PRO_FILE_PWD_/defaults.conf) $$shell_quote($$OUT_PWD)
