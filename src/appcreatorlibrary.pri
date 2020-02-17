include($$replace(_PRO_FILE_PWD_, ([^/]+$), \\1/\\1_dependencies.pri))
TARGET = $$QTC_LIB_NAME

include(../appcreator.pri)

win32{
DLLDESTDIR = $$IDE_BIN_PATH
}

DESTDIR = $$IDE_LIBRARY_PATH

include(rpath.pri)

TARGET = $$qtLibraryName($$TARGET)

TEMPLATE = lib

CONFIG += shared dll

contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols
