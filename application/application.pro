QT       += core gui multimedia sql
SHARE_APP_NAME = thebeat
CONFIG += c++17

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

unix:!macx {
    # Include the-libs build tools
    include(/usr/share/the-libs/pri/buildmaster.pri)

    DEFINES += SYSTEM_LIBRARY_DIRECTORY=\\\"$$[QT_INSTALL_LIBS]\\\"

    QT += thelib
    TARGET = thebeat

    CONFIG += link_pkgconfig
    PKGCONFIG += taglib

    LIBS += -L$$OUT_PWD/../libthebeat/ -lthebeat

    CI = $$(CI)

    if (isEmpty(CI)) {
        target.path = $$[QT_INSTALL_BINS]
    } else {
        message("Building on CI");
        target.path = /usr/bin
    }

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

    QT += winextras

    INCLUDEPATH += "C:/Program Files/thelibs/include" "C:/Program Files/theinstaller/include" "C:/Program Files (x86)/taglib/include"
    LIBS += -L"C:/Program Files/thelibs/lib" -L"C:/Program Files/theinstaller/lib" -lthe-libs -ltheinstaller -L"C:\Program Files (x86)\taglib\lib" -ltag
    RC_FILE = icon.rc
    TARGET = theBeat

    DEFINES += HAVE_THEINSTALLER

    SOURCES += \
        platformintegration/winplatformintegration.cpp

    HEADERS += \
        platformintegration/winplatformintegration.h

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
    common.cpp \
    controlstrip.cpp \
    currenttrackpopover.cpp \
    library/librarylistview.cpp \
    library/librarymanager.cpp \
    library/librarymodel.cpp \
    libraryerrorpopover.cpp \
    main.cpp \
    mainwindow.cpp \
    othersourceswidget.cpp \
    playlistmodel.cpp \
    pluginmanager.cpp \
    qtmultimedia/qtmultimediamediaitem.cpp \
    settingsdialog.cpp \
    trackswidget.cpp \
    userplaylistswidget.cpp

HEADERS += \
    artistsalbumswidget.h \
    common.h \
    controlstrip.h \
    currenttrackpopover.h \
    library/librarylistview.h \
    library/librarymanager.h \
    library/librarymodel.h \
    libraryerrorpopover.h \
    mainwindow.h \
    othersourceswidget.h \
    playlistmodel.h \
    pluginmanager.h \
    qtmultimedia/qtmultimediamediaitem.h \
    settingsdialog.h \
    trackswidget.h \
    userplaylistswidget.h

FORMS += \
    artistsalbumswidget.ui \
    controlstrip.ui \
    currenttrackpopover.ui \
    libraryerrorpopover.ui \
    mainwindow.ui \
    othersourceswidget.ui \
    settingsdialog.ui \
    trackswidget.ui \
    userplaylistswidget.ui

RESOURCES += \
    resources.qrc

INCLUDEPATH += $$PWD/../libthebeat
DEPENDPATH += $$PWD/../libthebeat

DISTFILES += \
    com.vicr123.thebeat.desktop \
    defaults.conf \
    icon.rc
