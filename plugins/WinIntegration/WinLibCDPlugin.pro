QT += gui widgets multimedia network concurrent

TEMPLATE = lib
CONFIG += plugin

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Link with CDLib and the-libs
CONFIG(release, debug|release): LIBS += -L$$PWD/cdlib/x64/Release
CONFIG(debug, debug|release): LIBS += -L$$PWD/cdlib/x64/Debug

LIBS += -L"C:/Program Files/thelibs/lib" -lthe-libs -lCDLib -lUser32 -lwindowsapp -lcomsuppwd -lshlwapi -L"C:\Program Files (x86)\taglib\lib" -ltag
INCLUDEPATH += $$PWD/cdlib/include/  "C:/Program Files/thelibs/include" "C:/Program Files (x86)/taglib/include"

SOURCES += \
    audiocdplayerthread.cpp \
    burn/cdtextcrcgenerator.cpp \
    burn/cdtextgenerator.cpp \
    burn/winburndaoimage.cpp \
    burn/winburnjob.cpp \
    burn/winburnjobwidget.cpp \
    burn/winburnmanager.cpp \
    burn/winburnpopover.cpp \
    burn/winburnprovider.cpp \
    cdchecker.cpp \
    diskwatcher.cpp \
    plugin.cpp \
    trackinfo.cpp \
    wincdmediaitem.cpp

HEADERS += \
    audiocdplayerthread.h \
    burn/cdtextcrcgenerator.h \
    burn/cdtextgenerator.h \
    burn/daoformatlocker.h \
    burn/winburndaoimage.h \
    burn/winburnjob.h \
    burn/winburnjobwidget.h \
    burn/winburnmanager.h \
    burn/winburnpopover.h \
    burn/winburnprovider.h \
    cdchecker.h \
    diskwatcher.h \
    plugin.h \
    trackinfo.h \
    wincdmediaitem.h

DISTFILES += WinLibCDPlugin.json

# Include the-libs build tools
include(C:/Program Files/thelibs/pri/buildmaster.pri)

include(../plugins.pri)

FORMS += \
    burn/winburnjobwidget.ui \
    burn/winburnpopover.ui \
    cdchecker.ui
