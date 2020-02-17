QTC_PLUGIN_NAME = ProjectExplorer

isEmpty(3RD_PATH) {
3RD_PATH = $$PWD/../../../../3rd
}

DSLPARSER_LIBRARY_TYPE = staticlib
include($${3RD_PATH}/mobata/libs/dslparser/dslparser/dslparser.pri)

QTC_LIB_DEPENDS += \
    ssh \
    utils
QTC_PLUGIN_DEPENDS += \
    locator \
    find \
    coreplugin \
    texteditor
QT *= network
