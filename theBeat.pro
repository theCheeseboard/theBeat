TEMPLATE = subdirs

libproj.subdir = libthebeat

pluginsproj.subdir = plugins
pluginsproj.depends = libproj

applicationproj.subdir = application
applicationproj.depends = libproj pluginsproj

SUBDIRS += \
    libproj \
    applicationproj \
    pluginsproj
