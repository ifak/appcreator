include(appcreator.pri)

#version check qt
!minQtVersion(5, 1, 0) {
    message("Cannot build App Creator with Qt version $${QT_VERSION}.")
    error("Use at least Qt 5.1.0.")
}

TEMPLATE  = subdirs
CONFIG += c++11
CONFIG   += ordered

SUBDIRS = \
    src \
    share \
    lib/qtcreator/qtcomponents

#unix:!macx:!isEmpty(copydata):SUBDIRS += bin

OTHER_FILES += dist/copyright_template.txt
