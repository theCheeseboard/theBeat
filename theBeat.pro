TEMPLATE = subdirs

libproj.subdir = libthebeat

applicationproj.subdir = application
applicationproj.depends = libproj

pluginsproj.subdir = plugins
pluginsproj.depends = libproj

SUBDIRS += \
    libproj \
    applicationproj \
    pluginsproj
