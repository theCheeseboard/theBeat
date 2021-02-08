QT += gui widgets multimedia

TEMPLATE = lib
CONFIG += plugin

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Include the-libs build tools
include(/usr/local/share/the-libs/pri/gentranslations.pri)

SOURCES += \
    airplaymanager.mm \
    avfoundationmediaitem.mm \
    avfoundationurlhandler.cpp \
    avplayerinstance.mm \
    plugin.cpp

HEADERS += \
    airplaymanager.h \
    avfoundationmediaitem.h \
    avfoundationurlhandler.h \
    avplayerinstance.h \
    plugin.h


include(../plugins.pri)

DISTFILES += \
    AvFoundationPlugin.json

INCLUDEPATH += "/usr/local/include/the-libs"
LIBS += -L/usr/local/lib -lthe-libs -framework AVFoundation -framework CoreMedia -framework AVKit
