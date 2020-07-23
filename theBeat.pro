TEMPLATE = subdirs

libproj.subdir = libthebeat

applicationproj.subdir = application
applicationproj.depends = libproj

SUBDIRS += \
    libproj \
    applicationproj
