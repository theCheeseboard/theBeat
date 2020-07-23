QT       += core gui multimedia sql
SHARE_APP_NAME = thebeat

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
unix:!macx {
    # Include the-libs build tools
    include(/usr/share/the-libs/pri/buildmaster.pri)

    QT += thelib
    TARGET = thebeat

    CONFIG += link_pkgconfig
    PKGCONFIG += taglib

    LIBS += -L$$OUT_PWD/../libthebeat/ -lthebeat

    target.path = $$[QT_INSTALL_BINS]

    desktop.path = /usr/share/applications
    desktop.files = com.vicr123.thebeat.desktop

    icon.path = /usr/share/icons/hicolor/scalable/apps/
    icon.files = icons/thebeat.svg

    defaults.files = defaults.conf
    defaults.path = /etc/theSuite/theBeat/

    INSTALLS += target desktop icon defaults
}

win32 {
    # Include the-libs build tools
    include(C:/Program Files/thelibs/pri/buildmaster.pri)

    # TODO: Link with taglib

    INCLUDEPATH += "C:/Program Files/thelibs/include"
    LIBS += -L"C:/Program Files/thelibs/lib" -lthe-libs
    RC_FILE = icon.rc
    TARGET = theBeat

    win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libthebeat/release/ -lthebeat
    else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libthebeat/debug/ -lthebeat
}

macx {
    # Include the-libs build tools
    include(/usr/local/share/the-libs/pri/buildmaster.pri)

    # TODO: Link with taglib

    QT += macextras
    LIBS += -framework CoreFoundation -framework AppKit
    QMAKE_INFO_PLIST = Info.plist

    LIBS += -L$$OUT_PWD/../libthebeat/ -lthebeat

    blueprint {
        TARGET = "theBeat Blueprint"
        ICON = icon-bp.icns
    } else {
        TARGET = "theBeat"
        ICON = icon.icns
    }

    INCLUDEPATH += "/usr/local/include/the-libs"
    LIBS += -L/usr/local/lib -lthe-libs

    QMAKE_POST_LINK += $$quote(cp $${PWD}/dmgicon.icns $${PWD}/app-dmg-background.png $${PWD}/node-appdmg-config*.json $${OUT_PWD}/..)
}

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    artistsalbumswidget.cpp \
    controlstrip.cpp \
    library/librarymanager.cpp \
    library/librarymodel.cpp \
    main.cpp \
    mainwindow.cpp \
    qtmultimedia/qtmultimediamediaitem.cpp \
    trackswidget.cpp

HEADERS += \
    artistsalbumswidget.h \
    controlstrip.h \
    library/librarymanager.h \
    library/librarymodel.h \
    mainwindow.h \
    qtmultimedia/qtmultimediamediaitem.h \
    trackswidget.h

FORMS += \
    artistsalbumswidget.ui \
    controlstrip.ui \
    mainwindow.ui \
    trackswidget.ui

RESOURCES += \
    resources.qrc

INCLUDEPATH += $$PWD/../libthebeat
DEPENDPATH += $$PWD/../libthebeat

DISTFILES += \
    com.vicr123.thebeat.desktop \
    defaults.conf \
    icon.rc
