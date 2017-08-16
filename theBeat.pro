#-------------------------------------------------
#
# Project created by QtCreator 2017-06-18T15:43:15
#
#-------------------------------------------------

QT       += core gui phonon4qt5 dbus x11extras thelib
LIBS     += -ltag -lX11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = thebeat
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    visualisationframe.cpp \
    playlistmodel.cpp \
    dbusadaptors.cpp \
    aboutwindow.cpp \
    librarymodel.cpp \
    librarymanagedialog.cpp \
    playlistlistwidget.cpp \
    nativeeventfilter.cpp

HEADERS += \
        mainwindow.h \
    visualisationframe.h \
    playlistmodel.h \
    dbusadaptors.h \
    aboutwindow.h \
    librarymodel.h \
    librarymanagedialog.h \
    playlistlistwidget.h \
    nativeeventfilter.h

FORMS += \
        mainwindow.ui \
    aboutwindow.ui \
    librarymanagedialog.ui

RESOURCES += \
    resources.qrc

DISTFILES += \
    thebeat.desktop \
    thebeat.svg

unix {
    target.path = /usr/bin

    appentry.path = /usr/share/applications
    appentry.files = thebeat.desktop

    icon.path = /usr/share/icons
    icon.files = thebeat.svg

    INSTALLS += target appentry icon
}
