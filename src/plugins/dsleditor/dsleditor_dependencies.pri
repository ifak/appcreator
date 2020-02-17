DSLEDITOR_VERSION=0.1

isEmpty(3RD_PATH) {
3RD_PATH = $$PWD/../../../../3rd
}

include($${3RD_PATH}/mobata/libs/dslparser/dslparser/dslparser.pri)

QTC_PLUGIN_NAME = dsleditor

QTC_LIB_DEPENDS += \
    aggregation \
    extensionsystem \
    languageutils \
    utils

QTC_PLUGIN_DEPENDS += \
    texteditor
